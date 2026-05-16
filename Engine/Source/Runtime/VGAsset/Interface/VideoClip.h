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
#include "../VGAssetConfig.h"
#include "VGCore/Include/Core/Core.h"
#include <NNMediaCore/Interface/VideoInterface.h>
#include <VGRHI/Interface/Texture.h>

#include "NNMediaCore/Include/FVideo.h"
#include "NNKernel/Interface/HCoreTypes.h"

namespace VisionGal {

	// 视频解码器接口
	struct IVideoClip : public VGEngineResource, public Horizon::IVideoClip {
		~IVideoClip() override = default;
	};

	class VG_ASSET_API FVideoClip : public IVideoClip
	{
	public:
		FVideoClip() = default;
		~FVideoClip() override = default;

		bool Open(const String& filePath);
		uint2 GetSize() const override;

		Horizon::IVideoDecoder* GetDecoder() override;
	private:
		Horizon::FVideoClip m_VideoClip;
	};
}
