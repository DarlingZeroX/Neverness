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
#include <NNMediaCore/Interface/AudioInterface.h>
#include "NNMediaCore/Include/Audio.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"

namespace NN::Runtime {

	// 音频解码器接口
	struct IAudioClip : public VGEngineResource, public NN::Core::IAudioClip {
		~IAudioClip() override = default;
	};

	struct VG_ASSET_API VGAudioClip: public IAudioClip
	{
		VGAudioClip() = default;
		~VGAudioClip() override = default;

		bool Open(const std::string& filePath);

		NN::Core::IAudioDecoder* GetDecoder() override;
	private:
		NN::Core::AudioClip m_AudioClip;
	};
}
