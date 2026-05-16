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
#include "VideoDecoderInterface.h"
#include <NNKernel/Interface/HConfig.h>
#include <NNKernel/Interface/HCoreTypes.h>
//#include "../../Graphics/Interface/Texture.h"

namespace Horizon {

	// 视频解码器接口
	struct IVideoClip
	{
		virtual ~IVideoClip() = default;

		virtual IVideoDecoder* GetDecoder() = 0;
		virtual uint2 GetSize() const = 0;
	};

	// 音频解码器接口
	struct IVideoPlayer {
		virtual ~IVideoPlayer() = default;

		virtual bool Open(const Ref<IVideoClip>& clip) { return false; };	// 打开音频片段
		virtual void Play() = 0;											// 播放视频
		virtual void Stop() = 0;											// 暂停播放
		virtual bool IsPlaying() const = 0;									// 是否正在播放视频
		virtual double GetDuration() const = 0;								// 获取视频时长
		virtual bool Pause() = 0;											// 暂停播放
		virtual bool Restore() = 0; 										// 恢复播放
		virtual void SetLoop(bool enable) = 0;								// 设置循环播放
		virtual bool IsLooping() const = 0;									// 是否循环播放
		virtual double GetPlaybackTime() const = 0;							// 获取当前播放时间（秒）
		virtual bool Seek(double seconds) = 0;								// 视频跳转
		virtual bool SetVolume(float v) = 0;								// 设置音量
		virtual float GetVolume() const = 0;
		virtual float GetVideoWidth() const = 0;
		virtual float GetVideoHeight() const = 0;
		virtual bool RestartPlay() = 0;
		virtual uint8_t* GetFrameData() = 0;

		virtual void Update() = 0;											// 更新
		//virtual VGFX::ITexture* GetVideoTexture() const = 0;				// 获取视频纹理
		//virtual Ref<VGFX::ITexture>GetVideoTextureRef() const = 0;				// 获取视频纹理
	};
}
