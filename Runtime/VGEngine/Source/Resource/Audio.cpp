#include "Resource/Audio.h"
#include "Core/VFS.h"

namespace VisionGal
{
	AudioClip::AudioClip()
        //audioSpec(),
        //audioBuffer(nullptr), bufferLength(0)
	{
	}

	AudioClip::~AudioClip()
	{
        Clear();
	}

    bool AudioClip::Open(const String& filePath)
    {
		// 初始化音频编解码器（如果尚未初始化）
		if (!SDL_WasInit(SDL_INIT_AUDIO)) {
			SDL_Init(SDL_INIT_AUDIO);
		}

        //auto result = VFS::SafeReadFileFromVFS(filePath, [&](const VFS::DataRef& data) {
        //    ioStream = SDL_IOFromMem(data->data(), data->size());
		//	audioData = data; // 保存音频数据引用，避免提前释放
        //    return 0;
        //    });

		return audioDecoder.Open(filePath);

        //if (result < 0)
        //    return false;

        return LoadAudio();
    }

    void AudioClip::Clear()
    {
        //if (audioBuffer) {
        //    SDL_free(audioBuffer);
        //    audioBuffer = nullptr;
        //}
    }

    bool AudioClip::LoadAudio()
    {
        // 释放旧数据
        //if (audioBuffer) {
        //    SDL_free(audioBuffer);
        //    audioBuffer = nullptr;
        //}
		//
        //// 加载音频文件
        //if (SDL_LoadWAV_IO(ioStream, 1, &audioSpec, &audioBuffer, &bufferLength) == false) {
        //    std::cerr << "加载WAV失败: " << SDL_GetError() << std::endl;
        //    return false;
        //}
		
        return true;
    }

    AudioPlayer::AudioPlayer()
        :
        audioStream(nullptr),
        isPlaying(false),
        bufferCopy(nullptr)
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
		//
        //// 复制数据用于播放
        //bufferCopy = new uint8_t[audioClip->bufferLength];
        //memcpy(bufferCopy, audioClip->audioBuffer, audioClip->bufferLength);
        //bufferPosition = bufferCopy;
        //remainingLength = audioClip->bufferLength;

        return true;
    }

    // 音频流回调函数
    static void AudioStreamCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        AudioPlayer* player = static_cast<AudioPlayer*>(userdata);
        player->HandleAudioStream(stream, additional_amount);
    }

    bool AudioPlayer::Play()
    {
        if (!audioClip || isPlaying) return false;

		audioClip->audioDecoder.PlayAudio();
        // 打开音频设备并创建流
        //audioStream = SDL_OpenAudioDeviceStream(
        //    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,  // 默认播放设备
        //    &audioClip->audioSpec,               // 音频规格
        //    AudioStreamCallback,                 // 回调函数
        //    this                                 // 用户数据
        //);
		//
        //if (!audioStream) {
        //    std::cerr << "创建音频流失败: " << SDL_GetError() << std::endl;
        //    return false;
        //}
		//
        //// 开始播放
        //SDL_ResumeAudioStreamDevice(audioStream);
        isPlaying = true;
        return true;
    }

    void AudioPlayer::SetLoop(bool enable)
    {
        m_EnableLoop = enable;
    }

    void AudioPlayer::Stop()
    {
        if (!isPlaying) return;
		audioClip->audioDecoder.Stop();

        // 暂停并销毁流
        //SDL_PauseAudioStreamDevice(audioStream);
        //SDL_DestroyAudioStream(audioStream);
        //audioStream = nullptr;
		//
        //// 释放复制缓冲区
        //if (bufferCopy) {
        //    delete[] bufferCopy;
        //    bufferCopy = nullptr;
        //}

        isPlaying = false;
    }

    bool AudioPlayer::IsPlayingAudio() const
    {
        return isPlaying;
    }

    bool AudioPlayer::IsLooping() const
    {
        return m_EnableLoop;
    }

    void AudioPlayer::SetVolume(float v)
    {
        m_Volume = SDL_clamp(v, 0.0f, 1.0f);
    }

    float AudioPlayer::GetVolume() const
    {
        return m_Volume;
    }

    void AudioPlayer::HandleAudioStream(SDL_AudioStream* stream, int need_bytes)
    {
        //if (remainingLength <= 0) {
        //    if (m_EnableLoop)
        //    {
        //        bufferPosition = bufferCopy;
        //        remainingLength = audioClip->bufferLength;
        //    }
        //    else
        //    {
        //        // 播放完毕，填充静音
        //        SDL_PauseAudioStreamDevice(stream);
        //        isPlaying = false;
        //        return;
        //    }
        //}
		//
        //// 计算可复制的数据量
        //int copy_bytes = SDL_min(remainingLength, need_bytes);
		//
        //// 如果音量 < 1，做音量缩放
        //if (m_Volume < 0.99f) {
        //    int16_t* samples = reinterpret_cast<int16_t*>(bufferPosition);
        //    size_t sampleCount = copy_bytes / sizeof(int16_t);
        //    std::vector<int16_t> temp(samples, samples + sampleCount);
		//
        //    for (auto& s : temp) {
        //        s = static_cast<int16_t>(s * m_Volume);
        //    }
        //    SDL_PutAudioStreamData(stream, temp.data(), copy_bytes);
        //}
        //else {
        //    // 无需调整
        //    SDL_PutAudioStreamData(stream, bufferPosition, copy_bytes);
        //}
        ////SDL_PutAudioStreamData(stream, bufferPosition, copy_bytes);
		//
        //// 更新位置和剩余长度
        //bufferPosition += copy_bytes;
        //remainingLength -= copy_bytes;
    }
}
