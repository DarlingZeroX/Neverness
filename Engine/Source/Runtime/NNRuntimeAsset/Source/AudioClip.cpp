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

#include "AudioClip.h"
//#include "Resource/Audio/FAudioDecoder.h"
#include "NNRuntimeCore/Include/Core/VFS.h"

namespace NN::Runtime
{
	bool VGAudioClip::Open(const std::string& filePath)
	{
		return m_AudioClip.Open(VFS::GetInstance(), filePath);

		return true;
	}

	NN::Core::IAudioDecoder* VGAudioClip::GetDecoder()
	{
		return m_AudioClip.GetDecoder();
	}
}
