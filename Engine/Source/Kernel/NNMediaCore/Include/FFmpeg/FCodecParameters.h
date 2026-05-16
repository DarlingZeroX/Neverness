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
//#include "../../Core/Core.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace Horizon {

	struct FfmpegAVCodecParameters
	{
		FfmpegAVCodecParameters(AVCodecParameters* param) 
		: m_Parameters(param)
		{
		} ;

		AVCodecParameters* m_Parameters = nullptr;
	};

}
