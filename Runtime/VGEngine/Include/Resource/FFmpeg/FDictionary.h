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

extern "C" {
#include <libavformat/avformat.h>
}

namespace VisionGal {

	struct FfmpegAVDictionary
	{
		FfmpegAVDictionary() = default;
		~FfmpegAVDictionary();

		void Set(const std::string& key, const std::string& value, int flags);

		AVDictionary* Get() const;
		AVDictionary** GetAddress();
	private:
		AVDictionary* m_Dictionary = nullptr;
	};
}
