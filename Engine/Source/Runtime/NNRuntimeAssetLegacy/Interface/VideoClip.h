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
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include <NNMediaCore/Interface/VideoInterface.h>
#include <NNRuntimeRHI/Interface/Texture.h>

#include "NNMediaCore/Include/FVideo.h"
#include "NNCore/Interface/HCoreTypes.h"

namespace NN::Runtime {

	// 视频解码器接口
	struct IVideoClip : public VGEngineResource, public NN::Core::IVideoClip {
		~IVideoClip() override = default;
	};

	class VG_ASSET_API FVideoClip : public IVideoClip
	{
	public:
		FVideoClip() = default;
		~FVideoClip() override = default;

		bool Open(const String& filePath);
		uint2 GetSize() const override;

		NN::Core::IVideoDecoder* GetDecoder() override;
	private:
		NN::Core::FVideoClip m_VideoClip;
	};
}
