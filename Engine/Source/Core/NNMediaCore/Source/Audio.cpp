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

#include "Audio.h"
#include "Audio/FAudioDecoder.h"

namespace NN::Core
{
	AudioClip::AudioClip()
		:m_AudioDecoder(nullptr)
	{
	}

	AudioClip::~AudioClip()
	{
		if (m_AudioDecoder == nullptr)
			return;

		m_AudioDecoder->StopDecode();
	}

    bool AudioClip::Open(NN::Runtime::VFS::VirtualFileSystemPtr& vfs, const std::string& filePath)
    {
		auto decoder = MakeRef<FAudioDecoder>();

		bool result = decoder->Open(vfs, filePath);
		if (result)
			m_AudioDecoder = decoder;

		return result;
    }

    IAudioDecoder* AudioClip::GetDecoder()
    {
		if (m_AudioDecoder)
			return m_AudioDecoder.get();

		return nullptr;
    }

    AudioPlayer::AudioPlayer()
        :
       // m_AudioStream(nullptr),
        m_IsPlaying(false)
    {

    }

    AudioPlayer::~AudioPlayer()
    {
        Stop();
    }

    Ref<AudioPlayer> AudioPlayer::CreatePlayer(const Ref<IAudioClip>& clip)
    {
		auto player = MakeRef<AudioPlayer>();
		player->OpenAudioClip(clip);
		return player;
    }

    bool AudioPlayer::OpenAudioClip(const Ref<IAudioClip>& clip)
    {
        m_AudioClip = clip;
        return true;
    }

    // 音频流回调函数
	void AudioPlayer::AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        AudioPlayer* player = static_cast<AudioPlayer*>(userdata);
		player->HandelAudioStream(stream, additional_amount, total_amount);
    }

    bool AudioPlayer::Play()
    {
        if (m_AudioClip == nullptr) 
			return false;

		if (m_IsPlaying)
			return true;

		m_AudioClip->GetDecoder()->StartDecode();

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

	bool AudioPlayer::Stop()
    {
		if (m_AudioClip == nullptr)
			return false;

		m_AudioClip->GetDecoder()->StopDecode();
		m_AudioStream.Clear();

        m_IsPlaying = false;
		m_IsFinished = true;
		return true;
    }

	bool AudioPlayer::IsStop()
	{
		return m_IsFinished;
	}

	bool AudioPlayer::IsPlaying() const
    {
        return m_IsPlaying;
    }

	bool AudioPlayer::SetLoop(bool enable)
	{
		if (m_AudioClip == nullptr)
			return false;

		m_IsLoopPlay = enable;
		return true;
	}

    bool AudioPlayer::IsLooping() const
    {
		if (m_AudioClip == nullptr)
			return false;

		return m_IsLoopPlay;
    }

    bool AudioPlayer::SetVolume(float v)
    {
        m_Volume = std::clamp(v, 0.0f, 1.0f);
		return true;
    }

    float AudioPlayer::GetVolume() const
    {
        return m_Volume;
    }

    bool AudioPlayer::Pause()
    {
		if (m_AudioClip == nullptr)
			return false;

		if (m_AudioStream.HasStream() == false)
			return false;

		m_AudioStream.PauseStream();
		//SDL_PauseAudioStreamDevice(m_AudioStream);
		m_AudioClip->GetDecoder()->PauseDecode(true);
		m_IsPlaying = false;
		return true;
    }

    bool AudioPlayer::Restore()
    {
		if (m_AudioClip == nullptr)
			return false;

		if (m_AudioStream.HasStream() == false)
			return false;

		auto* audioBuffer = m_AudioClip->GetDecoder()->GetAudioBuffer();
		if (m_IsFinished)
		{
			audioBuffer->Reset();
			m_PlayedBytes = 0.f;
		}

		// 恢复音频流
		m_AudioStream.ResumeStream();

		// 先取消暂停解码
		m_AudioClip->GetDecoder()->PauseDecode(false);
		if (m_AudioClip->GetDecoder()->IsRunningDecode() == false)
		{
			// 循环就恢复解码
			if (IsLooping() == true)
			{
				m_AudioClip->GetDecoder()->RestoreDecode();
			}

			// 重新开始解码
			if (m_IsFinished)
			{
				m_AudioClip->GetDecoder()->RestartDecode();
				//RestartPlay();
			}
		}

		m_IsPlaying = true;
		m_IsFinished = false;
		return true;
    }

    double AudioPlayer::GetDuration() const
    {
		if (m_AudioClip == nullptr)
			return 0.f;

		return m_AudioClip->GetDecoder()->GetDuration();
    }

    double AudioPlayer::GetPlaybackTime() const
    {
		if (m_BytesPerSec <= 0)
			return 0.0;

		return static_cast<double>(m_PlayedBytes) / static_cast<double>(m_BytesPerSec);
    }

    bool AudioPlayer::RestartPlay()
    {
		m_PlayedBytes = 0;
		m_IsPlaying = true;
		m_IsFinished = false;
		Seek(0);
		Restore();
		//m_AudioClip->GetDecoder()->RestartDecode();


		return true;
    }

    bool AudioPlayer::Seek(double seconds)
	{
		if (!m_AudioClip || !m_AudioClip->GetDecoder())
			return false;

		auto decoder = m_AudioClip->GetDecoder();

		// 1. 暂停播放
		bool isPause = !IsPlaying();
		if (m_IsPlaying)
			Pause();

		// 2. 告诉 decoder 跳转
		if (!decoder->Seek(seconds)) {
			std::cerr << "[AudioPlayer] Seek failed at " << seconds << "s\n";
			return false;
		}

		// 4. 重置已播放字节数
		m_PlayedBytes = static_cast<size_t>(seconds * m_BytesPerSec);

		// 5. 如何没有暂停恢复播放
		if (isPause == false)
			Restore();

		return true;
	}

    void AudioPlayer::FinishPlay(SDL_AudioStream* stream)
    {
		if (m_AudioClip == nullptr)
			return;

		// 如果开启了循环播放，就重新开始
		if (m_IsLoopPlay)
		{
			RestartPlay();
		}
		else  // 正常结束
		{
			SDL_PauseAudioStreamDevice(stream);
			m_IsPlaying = false;
			m_IsFinished = true;
		}
    }

    void AudioPlayer::HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount)
    {
		auto* ring = this->m_AudioClip->GetDecoder()->GetAudioBuffer();
		size_t frame_size = 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); // 2ch s16

		while (additional_amount >= static_cast<int>(frame_size) && ring->Available() >= frame_size) {
			uint8_t temp[4096];
			size_t to_read = std::min(sizeof(temp), static_cast<size_t>(additional_amount));

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
			FinishPlay(stream);
			return;
		}

		if (ring->Available() < frame_size * 10) {
			if (ring->IsWriteFinish())
			{
				FinishPlay(stream);
				return;
			}
			else
			{
				std::cerr << "[Audio] Warning: underrun imminent, ring buffer low" << std::endl;
			}
		}
    }
}
