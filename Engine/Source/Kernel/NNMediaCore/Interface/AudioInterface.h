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
#include "AudioDecoderInterface.h"
#include <NNKernel/Interface/HCore.h>

namespace Horizon {

	// 音频解码器接口
	struct IAudioClip
	{
		virtual ~IAudioClip() = default;

		virtual IAudioDecoder* GetDecoder() = 0;
	};

	// 音频解码器接口
	struct IAudioPlayer {
		virtual ~IAudioPlayer() = default;

		virtual bool OpenAudioClip(const Ref<IAudioClip>& clip) = 0;	// 打开音频片段

		virtual bool Play() = 0;										// 开始播放
		virtual bool SetLoop(bool enable) = 0;							// 循环播放
		virtual bool Stop() = 0;										// 停止播放
		virtual bool IsStop() = 0;
		virtual bool IsPlaying() const = 0;								// 是否正在播放音频
		virtual bool IsLooping() const = 0;								// 是否循环播放
		virtual bool SetVolume(float v) = 0;							// 设置音量
		virtual float GetVolume() const = 0;							// 获取音量
		virtual bool Pause() = 0;										// 暂停播放
		virtual bool Restore() = 0;										// 恢复播放
		virtual double GetDuration() const = 0;
		virtual double GetPlaybackTime() const = 0;				// 获取音频设备的当前播放时间（秒）

		virtual bool Seek(double seconds) = 0;
	};
}
