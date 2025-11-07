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

namespace VisionGal
{
	AudioClip::AudioClip()
	{
	}

	AudioClip::~AudioClip()
	{
	}

    bool AudioClip::Open(const String& filePath)
    {
		// 初始化音频编解码器（如果尚未初始化）
		if (!SDL_WasInit(SDL_INIT_AUDIO)) {
			SDL_Init(SDL_INIT_AUDIO);
		}

		return audioDecoder.Open(filePath);
    }

    AudioPlayer::AudioPlayer()
        :
        audioStream(nullptr),
        isPlaying(false)
    {
        Init();
    }

    AudioPlayer::~AudioPlayer()
    {
        //Stop();
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
        audioClip = clip;

        //Stop();

        return true;
    }

    // 音频流回调函数
    static void AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        AudioPlayer* player = static_cast<AudioPlayer*>(userdata);
        //player->HandleAudioStream(stream, additional_amount);
		player->HandelAudioStream(stream, additional_amount, total_amount);
    }

    bool AudioPlayer::Play()
    {
		//if (audioClip->audioDecoder == nullptr)
		//	return false;

        if (!audioClip || isPlaying) return false;

		audioClip->audioDecoder.StartDecode();


		if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false) {
			std::cerr << "音频初始化失败: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_AudioSpec spec{};
		spec.freq = 44100;
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;

		// 1. 打开默认输出设备
		audioDev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
		if (!audioDev) {
			std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
			return false;
		}

		audioStream = SDL_OpenAudioDeviceStream(audioDev, &spec, AudioStreamCallback, this);
		if (!audioStream) {
			std::cerr << "Failed to create audio stream: " << SDL_GetError() << std::endl;
			return false;
		}

		// 3. 开始播放
		SDL_ResumeAudioStreamDevice(audioStream);

        isPlaying = true;
        return true;
    }

    void AudioPlayer::Stop()
    {
		if (isPlaying == false)
			return;

		audioClip->audioDecoder.StopDecode();

		//SDL_PauseAudioStreamDevice(audioStream);
		// 暂停并销毁流
		if (audioStream != nullptr)
		{
			SDL_PauseAudioStreamDevice(audioStream);
			SDL_DestroyAudioStream(audioStream);
			audioStream = nullptr;
		}

        isPlaying = false;
    }

    bool AudioPlayer::IsPlayingAudio() const
    {
        return isPlaying;
    }

	void AudioPlayer::SetLoop(bool enable)
	{
		if (audioClip)
		{
			audioClip->audioDecoder.SetDecodeLoop(enable);
		}
	}

    bool AudioPlayer::IsLooping() const
    {
		if (audioClip)
		{
			return audioClip->audioDecoder.IsDecodeLoop();
		}

		return false;
        //return m_EnableLoop;
    }

    void AudioPlayer::SetVolume(float v)
    {
        m_Volume = SDL_clamp(v, 0.0f, 1.0f);
    }

    float AudioPlayer::GetVolume() const
    {
        return m_Volume;
    }

    void AudioPlayer::FinishPlay(SDL_AudioStream* stream)
    {
		SDL_PauseAudioStreamDevice(stream);
		isPlaying = false;
    }

    void AudioPlayer::HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount)
    {
		//auto* ring = static_cast<AudioRingBuffer*>(userdata);
		auto* ring = this->audioClip->audioDecoder.GetAudioBuffer();
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
