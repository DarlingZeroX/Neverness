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

#pragma once
#include "../Interface/AudioInterface.h"
#include "SDL/SDLAudio.h"
#include "HMediaConfig.h"
#include <NNKernel/Interface/HCore.h>
#include <NNFileSystem/Include/VFS/VirtualFileSystem.hpp>

namespace Horizon
{
    class H_MEDIA_API AudioClip : public IAudioClip
    {
    public:
        AudioClip();
        ~AudioClip() override;

        bool Open(vfspp::VirtualFileSystemPtr& vfs, const std::string& filePath);

		IAudioDecoder* GetDecoder() override;
    private:
		Ref<IAudioDecoder> m_AudioDecoder;
    };
	
    class H_MEDIA_API AudioPlayer : public IAudioPlayer
	{
        static void AudioStreamCallback(void*, SDL_AudioStream*, int, int);
    public:
        AudioPlayer();
        ~AudioPlayer() override;

		static Ref<AudioPlayer> CreatePlayer(const Ref<IAudioClip>& clip);

        bool OpenAudioClip(const Ref<IAudioClip>& clip) override;	        // 打开音频片段
        bool Play() override;												// 开始播放
		bool SetLoop(bool enable) override;									// 循环播放
        bool Stop() override;												// 停止播放
		bool IsStop() override;												// 是否停止播放了
        bool IsPlaying() const override;									// 是否正在播放音频
        bool IsLooping() const override;									// 是否循环播放
		bool SetVolume(float v) override;									// 设置音量
        float GetVolume() const override;									// 获取音量
		bool Pause() override;												// 暂停播放
		bool Restore() override;											// 恢复播放
		double GetDuration() const override;								// 获取总播放时长
		double GetPlaybackTime() const override;							// 获取视频设备的当前播放时间（秒）
		bool RestartPlay();

		bool Seek(double seconds) override;
    private:
		void FinishPlay(SDL_AudioStream* stream);
        // 处理音频流数据
		void HandelAudioStream(SDL_AudioStream* stream, int additional_amount, int total_amount);

        Ref<IAudioClip> m_AudioClip = nullptr;
		SDL3AudioStream m_AudioStream;
		SDL3AudioDevice m_AudioDevice;

		// 状态
		bool m_IsFinished = false;
        bool m_IsPlaying = false;
		bool m_IsLoopPlay = false;
        float m_Volume = 1.0f;
		size_t m_PlayedBytes = 0;    // 已经被 SDL 播放的 PCM 字节数
		int    m_BytesPerSec = 0;    // 每秒 PCM 字节数（根据 format 计算）
    };
	
}
