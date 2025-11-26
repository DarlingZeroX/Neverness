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

#include "FFmpeg/FDictionary.h"

namespace Horizon {

	FfmpegAVDictionary::~FfmpegAVDictionary()
	{
		// 释放选项字典
		av_dict_free(&m_Dictionary);
	}

	void FfmpegAVDictionary::Set(const std::string& key, const std::string& value, int flags)
	{
		av_dict_set(&m_Dictionary, key.c_str(), value.c_str(), 0); // 最大int值
	}

	AVDictionary* FfmpegAVDictionary::Get() const
	{
		return m_Dictionary;
	}

	AVDictionary** FfmpegAVDictionary::GetAddress()
	{
		return &m_Dictionary;
	}

}