/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "FVideo.h"
#include <SDL3/SDL_audio.h>
#include "Video/FVideoDecoder.h"
#include <NNCore/Interface/HVector.h>

namespace NN::Core
{
	////////////////////	FVideoClip
	uint2 FVideoClip::GetSize() const
	{
		return { m_VideoDecoder->GetVideoWidth(),m_VideoDecoder->GetVideoHeight() };
	}

	IVideoDecoder* FVideoClip::GetDecoder()
	{
		return m_VideoDecoder.get();
	}

	////////////////////	FVideoPlayer
	bool FVideoPlayer::PlayAudio()
	{
		if (m_VideoClip == nullptr)
			return false;

		if (m_VideoClip->GetDecoder()->HasAudioStream() == false)
			return false;

		SDL_AudioSpec spec{};
		spec.freq = 44100;
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;

		// 1. 初始化 m_BytesPerSec
		m_BytesPerSec = spec.freq * spec.channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
		m_PlayedBytes = 0;   // 播放重置

		// 2. 打开默认输出设备
		if (m_AudioDevice.OpenDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, spec) == false)
			return false;

		// 3. 打开音频流
		if (m_AudioStream.OpenStream(m_AudioDevice, AudioStreamCallback, this) == false)
			return false;

		// 4. 开始播放音频流
		if (m_AudioStream.ResumeStream() == false)
			return false;

		m_IsPlaying = true;
		return true;
	}

	void FVideoPlayer::ProcessVideoUpdate(uint8_t* frameData, int& linesize, int width, int height, double pts)
	{
		if (!m_IsPlaying)
			return;

		// 视频开始时初始化基准时间
		if (!m_VideoClockInitialized) {
			m_VideoClockInitialized = true;
			m_SystemStartTime = std::chrono::steady_clock::now();
			m_VideoStartPTS = pts;          // 通常是 0
		}

		// 计算从视频开头起，应该显示这帧的时间（秒）
		double targetTime = pts - m_VideoStartPTS;

		// 当前系统已经过去的时间（秒）
		auto now = std::chrono::steady_clock::now();
		double elapsed = std::chrono::duration<double>(now - m_SystemStartTime).count();

		//std::cout << "[Play] videoPts " << pts << "s AudioClock=" << decoder->GetAudioClock() << ")\n";

		// 如果视频太快，就睡眠
		if (elapsed < targetTime) {
			std::this_thread::sleep_for(std::chrono::duration<double>(targetTime - elapsed));
		}

		// 在这里更新纹理 / 显示图像
		{
			std::unique_lock lock(m_Mutex);
			m_FrameData = frameData;
		}
		//m_LastPts = pts;
	}

	void FVideoPlayer::FinishPlayAudio(SDL_AudioStream* stream)
	{
		SDL_PauseAudioStreamDevice(stream);
	}

	void FVideoPlayer::HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount)
	{
		if (m_VideoClip == nullptr)
			return;

		//auto* ring = static_cast<AudioRingBuffer*>(userdata);
		auto* ring = this->m_VideoClip->GetDecoder()->GetAudioBuffer();
		size_t frame_size = 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); // 2ch s16

		while (additional_amount >= (int)frame_size && ring->Available() >= frame_size) {
			uint8_t temp[4096];
			size_t to_read = std::min(sizeof(temp), (size_t)additional_amount);

			// 保证读取的是完整帧数
			to_read = (to_read / frame_size) * frame_size;

			if (to_read == 0) break;

			size_t read = ring->Read(temp, to_read);

			// 再次对齐防止 AudioRingBuffer 只返回部分
			read = (read / frame_size) * frame_size;

			// 播放的字节
			m_PlayedBytes += read;

			if (read > 0) {
				// 音量调整
				if (m_Volume < 0.99f) {
					int16_t* samples = reinterpret_cast<int16_t*>(temp);
					size_t sampleCount = read / sizeof(int16_t);
					std::vector<int16_t> adjustedSamples(samples, samples + sampleCount);
					for (auto& s : adjustedSamples) {
						s = static_cast<int16_t>(s * m_Volume);
					}
					SDL_PutAudioStreamData(stream, adjustedSamples.data(), (int)read);
				}
				else {
					// 无需调整
					SDL_PutAudioStreamData(stream, temp, (int)read);
				}

				//SDL_PutAudioStreamData(stream, temp, (int)read);
				additional_amount -= (int)read;
			}
			else {
				break;
			}
		}

		if (ring->IsFinish())
		{
			FinishPlayAudio(stream);
			//SDL_PauseAudioStreamDevice(stream);
			return;
		}

		if (ring->Available() < frame_size * 10) {
			if (ring->IsWriteFinish())
			{
				FinishPlayAudio(stream);
				//SDL_PauseAudioStreamDevice(stream);
				return;
			}
			else
			{
				std::cerr << "[Audio] Warning: underrun imminent, ring buffer low" << std::endl;
			}
		}
	}

	bool FVideoPlayer::IsLooping() const
	{
		return m_IsLoopPlay;
	}

	void FVideoPlayer::SetLoop(bool loop)
	{
		m_IsLoopPlay = loop;
	}

	bool FVideoClip::Open(NN::Runtime::VFS::VirtualFileSystemPtr& vfs, const std::string& filePath)
	{
		auto decoder = MakeRef<FVideoDecoder>();

		bool result = decoder->Open(vfs, filePath);
		if (result)
			m_VideoDecoder = decoder;

		return result;
	}

	void FVideoPlayer::AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
	{
		FVideoPlayer* player = static_cast<FVideoPlayer*>(userdata);
		//player->HandleAudioStream(stream, additional_amount);
		player->HandelAudioStream(stream, additional_amount, total_amount);
	}

	FVideoPlayer::FVideoPlayer()
		:
		m_IsPlaying(false) {
	}

	FVideoPlayer::~FVideoPlayer()
	{
		Stop();

		if (m_VideoClip != nullptr)
		{
			m_VideoClip = nullptr;
		}
	}

	Ref<FVideoPlayer> FVideoPlayer::CreatePlayer(const Ref<IVideoClip>& clip)
	{
		auto player = MakeRef<FVideoPlayer>();
		player->Open(clip);
		return player;
	}

	bool FVideoPlayer::Open(const Ref<IVideoClip>& clip)
	{
		Stop();
		m_VideoClip = clip;

		//VGFX::TextureDesc desc;
		//desc.Width = m_VideoClip->GetDecoder()->GetVideoWidth();
		//desc.Height = m_VideoClip->GetDecoder()->GetVideoHeight();
		//desc.Type = GL_UNSIGNED_BYTE;
		//desc.Format = GL_RGBA;
		//desc.InternalFormat = GL_RGBA;
		//desc.Data = nullptr;
		//m_VideoTexture = VGFX::CreateTextureFromMemory(desc);

		return true;
	}

	void FVideoPlayer::Play()
	{
		if (m_IsPlaying) return;

		m_VideoClip->GetDecoder()->OnVideoDataUpdate = [this](uint8_t* frameData, int& linesize, int width, int height, double pts)
			{
				ProcessVideoUpdate(frameData, linesize, width, height, pts);
			};

		m_VideoClip->GetDecoder()->StartDecode();
		//decoder->PlayAudio();
		PlayAudio();

		m_IsPlaying = true;
		// playbackThread = std::thread(&VideoPlayer::PlayBackLoop, this);
	}

	void FVideoPlayer::Stop()
	{
		if (m_VideoClip)
			m_VideoClip->GetDecoder()->StopDecode();
		m_IsPlaying = false;

		// 销毁音频
		m_AudioStream.Clear();

		m_IsPlaying = false;
		//m_IsFinished = true;
	}

	bool FVideoPlayer::IsPlaying() const
	{
		return m_IsPlaying;// && !renderer->shouldClose();
	}

	double FVideoPlayer::GetDuration() const
	{
		if (m_VideoClip == nullptr)
			return 0.f;

		return m_VideoClip->GetDecoder()->GetDuration();
	}

	bool FVideoPlayer::Pause()
	{
		if (m_VideoClip == nullptr)
			return false;

		m_AudioStream.PauseStream();
		//if (m_AudioStream != nullptr)
		//{
		//	SDL_PauseAudioStreamDevice(m_AudioStream);
		//}

		m_VideoClip->GetDecoder()->PauseDecode(true);
		m_IsPlaying = false;
		return true;
	}

	bool FVideoPlayer::Restore()
	{
		if (m_VideoClip == nullptr)
			return false;

		if (m_VideoClip->GetDecoder()->IsFinishedDecode())
		{
			m_VideoClockInitialized = false;
		}

		if (m_VideoClip->GetDecoder()->HasAudioStream())
		{
			auto* audioBuffer = m_VideoClip->GetDecoder()->GetAudioBuffer();
			if (m_VideoClip->GetDecoder()->IsFinishedDecode())
			{
				audioBuffer->Reset();
				m_PlayedBytes = 0.f;
			}
		}

		m_AudioStream.ResumeStream();
		//if (m_AudioStream != nullptr)
		//	SDL_ResumeAudioStreamDevice(m_AudioStream);
		m_VideoClip->GetDecoder()->PauseDecode(false);
		if (m_VideoClip->GetDecoder()->IsRunningDecode() == false)
		{
			if (m_VideoClip->GetDecoder()->IsFinishedDecode())
			{
				//RestartPlay();
				m_VideoClip->GetDecoder()->RestartDecode();
			}
		}

		m_VideoClip->GetDecoder()->RestoreDecode();
		m_IsPlaying = true;
		//m_IsFinished = false;
		return true;
	}

	void FVideoPlayer::Update()
	{
		if (!m_IsPlaying)
			return;

		if (m_FrameData == nullptr)
			return;

		if (m_VideoClip->GetDecoder()->IsFinishedDecode())
		{
			m_IsPlaying = false;
			if (m_IsLoopPlay)
			{
				RestartPlay();
				return;
			}
		}

		//int textureID = reinterpret_cast<int>(m_VideoTexture->GetShaderResourceView());
		//
		//// 更新纹理
		//GL_THROW_INFO( glBindTexture(GL_TEXTURE_2D, textureID) );
		//GL_THROW_INFO( glTexImage2D(
		//	GL_TEXTURE_2D, 0, GL_RGBA,
		//	m_VideoTexture->GetDesc().Width,
		//	m_VideoTexture->GetDesc().Height,
		//	0, GL_RGBA, GL_UNSIGNED_BYTE,
		//	m_FrameData
		//));
		//
		//GL_THROW_INFO(glBindTexture(GL_TEXTURE_2D, 0));
	}

	double FVideoPlayer::GetPlaybackTime() const
	{
		if (m_VideoClip->GetDecoder()->HasAudioStream())
		{
			if (m_BytesPerSec <= 0)
				return 0.0;

			return static_cast<double>(m_PlayedBytes) / static_cast<double>(m_BytesPerSec);
		}
		else
		{
			return m_VideoClip->GetDecoder()->GetPlaybackTime();
		}
	}

	bool FVideoPlayer::Seek(double seconds)
	{
		if (!m_VideoClip || !m_VideoClip->GetDecoder())
			return false;

		auto decoder = m_VideoClip->GetDecoder();

		m_VideoClockInitialized = false;

		bool isPlaying = IsPlaying();
		// 1. 暂停播放
		//if (m_IsPlaying)
		Pause();

		// 2. 告诉 decoder 跳转
		if (!decoder->Seek(seconds)) {
			std::cerr << "[AudioPlayer] Seek failed at " << seconds << "s\n";
			return false;
		}

		// 4. 重置已播放字节数
		m_PlayedBytes = size_t(seconds * m_BytesPerSec);

		// 5. 恢复播放
		if (isPlaying == true)
			Restore();

		return true;
	}

	bool FVideoPlayer::RestartPlay()
	{
		m_IsPlaying = true;
		Seek(0);

		//m_PlayedBytes = 0;
		//Restore();
		//m_IsPlaying = true;
		//m_VideoClockInitialized = false;
		////m_IsFinished = false;
		//m_VideoClip->GetDecoder()->RestartDecode();

		return true;
	}

	bool FVideoPlayer::SetVolume(float v)
	{
		m_Volume = std::clamp(v, 0.0f, 1.0f);
		return true;
	}

	float FVideoPlayer::GetVolume() const
	{
		return m_Volume;
	}

	float FVideoPlayer::GetVideoWidth() const
	{
		return m_VideoClip->GetSize().x;
	}

	float FVideoPlayer::GetVideoHeight() const
	{
		return m_VideoClip->GetSize().y;
	}

	uint8_t* FVideoPlayer::GetFrameData()
	{
		return m_FrameData;
	}
}
