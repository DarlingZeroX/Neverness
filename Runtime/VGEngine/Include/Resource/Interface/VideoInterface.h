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
#include "../../Core/Core.h"
#include <HMedia/Interface/VideoInterface.h>
#include <VGRHI/Interface/Texture.h>

namespace VisionGal {

	// 视频解码器接口
	struct IVideoClip : public VGEngineResource, public Horizon::IVideoClip {
		~IVideoClip() override = default;
	};

	// 音频解码器接口
	struct IVideoPlayer: public Horizon::IVideoPlayer{
		~IVideoPlayer() override = default;

		virtual bool Open(const Ref<IVideoClip>& clip) = 0;					// 打开音频片段
		//
		//virtual void Update() = 0;											// 更新
		virtual VGFX::ITexture* GetVideoTexture() const = 0;				// 获取视频纹理
		virtual Ref<VGFX::ITexture>GetVideoTextureRef() const = 0;				// 获取视频纹理
	};
}
