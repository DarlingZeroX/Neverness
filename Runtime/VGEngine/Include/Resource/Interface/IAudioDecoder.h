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
#include "IAudioDataBuffer.h"

namespace VisionGal {

	// 音频解码器接口
	struct IAudioDecoder {
		virtual ~IAudioDecoder() = default;

		virtual void Close() = 0;
		virtual double GetDuration() const = 0;

		virtual IAudioDataBuffer* GetAudioBuffer() const = 0;
		virtual double GetAudioClock() const = 0;

		virtual void StartDecode() = 0;					// 开始解码
		virtual void StopDecode() = 0;					// 暂停解码
		virtual void SetLoopDecode(bool enable) = 0;	// 设置循环解码
		virtual bool IsLoopDecode() const = 0;			// 是否循环解码
		virtual void SetPauseDecode(bool pause) = 0;	// 设置暂停解码
		virtual bool IsPauseDecode() const = 0;			// 是否暂停解码
	};
}
