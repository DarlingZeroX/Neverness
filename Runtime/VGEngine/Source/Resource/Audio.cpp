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

#include "Resource/Audio.h"
#include "Core/VFS.h"
#include "Resource/Audio/FAudioDecoder.h"

namespace VisionGal
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

    bool AudioClip::Open(const String& filePath)
    {
		auto decoder = CreateRef<FAudioDecoder>();

		bool result = decoder->Open(filePath);
		if (result)
			m_AudioDecoder = decoder;

		return result;
    }

    IAudioDecoder* AudioClip::GetDecoder() const
    {
		if (m_AudioDecoder)
			return m_AudioDecoder.get();

		return nullptr;
    }

    AudioPlayer::AudioPlayer()
        :
        m_AudioStream(nullptr),
        m_IsPlaying(false)
    {
        Init();
    }

    AudioPlayer::~AudioPlayer()
    {
        Stop();
    }

    Ref<AudioPlayer> AudioPlayer::CreatePlayer(const Ref<AudioClip>& clip)
    {
		auto player = CreateRef<AudioPlayer>();
		player->OpenAudioClip(clip);
		return player;
    }

    bool AudioPlayer::Init()
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
            std::cerr << "音频初始化失败: " << SDL_GetError() << std::endl;
            return false;
        }
        return true;
    }

    bool AudioPlayer::OpenAudioClip(const Ref<AudioClip>& clip)
    {
        m_AudioClip = clip;

        //Stop();

        return true;
    }

    // 音频流回调函数
	void AudioPlayer::AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        AudioPlayer* player = static_cast<AudioPlayer*>(userdata);
        //player->HandleAudioStream(stream, additional_amount);
		player->HandelAudioStream(stream, additional_amount, total_amount);
    }

    bool AudioPlayer::Play()
    {
		//if (audioClip->audioDecoder == nullptr)
		//	return false;

        if (m_AudioClip == nullptr) 
			return false;

		if (m_IsPlaying)
			return true;

		m_AudioClip->GetDecoder()->StartDecode();

		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
			std::cerr << "音频初始化失败: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_AudioSpec spec{};
		spec.freq = 44100;
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;

		// 初始化 m_BytesPerSec
		m_BytesPerSec = spec.freq * spec.channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
		m_PlayedBytes = 0;   // 播放重置

		// 1. 打开默认输出设备
		m_AudioDev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
		if (!m_AudioDev) {
			H_LOG_ERROR("Failed to open audio device: %s", SDL_GetError());
			return false;
		}

		// 2. 打开音频流
		m_AudioStream = SDL_OpenAudioDeviceStream(m_AudioDev, &spec, AudioStreamCallback, this);
		if (!m_AudioStream) {
			H_LOG_ERROR("Failed to create audio stream: %s", SDL_GetError());
			return false;
		}

		// 3. 开始播放
		SDL_ResumeAudioStreamDevice(m_AudioStream);

        m_IsPlaying = true;
        return true;
    }

	bool AudioPlayer::Stop()
    {
		if (m_AudioClip == nullptr)
			return false;

		m_AudioClip->GetDecoder()->StopDecode();

		if (m_IsPlaying == false)
			return true;

		//SDL_PauseAudioStreamDevice(audioStream);
		// 暂停并销毁流
		if (m_AudioStream != nullptr)
		{
			SDL_PauseAudioStreamDevice(m_AudioStream);
			SDL_DestroyAudioStream(m_AudioStream);
			m_AudioStream = nullptr;
		}

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

		m_AudioClip->GetDecoder()->SetLoopDecode(enable);
		return true;
	}

    bool AudioPlayer::IsLooping() const
    {
		if (m_AudioClip)
		{
			return m_AudioClip->GetDecoder()->IsLoopDecode();
		}

		return false;
        //return m_EnableLoop;
    }

    bool AudioPlayer::SetVolume(float v)
    {
        m_Volume = SDL_clamp(v, 0.0f, 1.0f);
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

		if (m_AudioStream == nullptr)
			return false;

		SDL_PauseAudioStreamDevice(m_AudioStream);
		m_AudioClip->GetDecoder()->PauseDecode();
		m_IsPlaying = false;
		return true;
    }

    bool AudioPlayer::Restore()
    {
		if (m_AudioClip == nullptr)
			return false;

		if (m_AudioStream == nullptr)
			return false;

		auto* audioBuffer = m_AudioClip->GetDecoder()->GetAudioBuffer();
		if (m_IsFinished)
		{
			audioBuffer->Reset();
			m_PlayedBytes = 0.f;
		}

		SDL_ResumeAudioStreamDevice(m_AudioStream);
		m_AudioClip->GetDecoder()->RestoreDecode();
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

    double AudioPlayer::GetAudioPlaybackTime() const
    {
		if (m_BytesPerSec <= 0)
			return 0.0;

		return static_cast<double>(m_PlayedBytes) / static_cast<double>(m_BytesPerSec);
    }

    bool AudioPlayer::Seek(double seconds)
	{
		if (!m_AudioClip || !m_AudioClip->GetDecoder())
			return false;

		auto decoder = m_AudioClip->GetDecoder();

		// 1. 暂停播放
		if (m_IsPlaying)
			Pause();

		// 2. 告诉 decoder 跳转
		if (!decoder->Seek(seconds)) {
			std::cerr << "[AudioPlayer] Seek failed at " << seconds << "s\n";
			return false;
		}

		// 3. 清空 RingBuffer
		auto ring = decoder->GetAudioBuffer();
		ring->Reset();

		// 4. 重置已播放字节数
		m_PlayedBytes = size_t(seconds * m_BytesPerSec);

		// 5. 恢复播放
		if (m_IsPlaying == false)
			Restore();

		return true;
	}

    void AudioPlayer::FinishPlay(SDL_AudioStream* stream)
    {
		SDL_PauseAudioStreamDevice(stream);
		m_IsPlaying = false;
		m_IsFinished = true;
    }

    void AudioPlayer::HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount)
    {
		//auto* ring = static_cast<AudioRingBuffer*>(userdata);
		auto* ring = this->m_AudioClip->GetDecoder()->GetAudioBuffer();
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
			FinishPlay(stream);
			//SDL_PauseAudioStreamDevice(stream);
			return;
		}

		if (ring->Available() < frame_size * 10) {
			if (ring->IsWriteFinish())
			{
				FinishPlay(stream);
				//SDL_PauseAudioStreamDevice(stream);
				return;
			}
			else
			{
				std::cerr << "[Audio] Warning: underrun imminent, ring buffer low" << std::endl;
			}
		}
    }

}
