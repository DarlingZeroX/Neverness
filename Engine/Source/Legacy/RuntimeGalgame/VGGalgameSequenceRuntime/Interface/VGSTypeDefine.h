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

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "../GSSExport.h"
#include <NNCore/Interface/HConfig.h>

namespace VisionGal
{
	/// 无效对象 ID：永不作为有效注册结果返回；外部不得将其解释为有效 Handle。
	inline constexpr uint32_t VGSS_INVALID_OBJECT_ID = 0u;

	using IVGSSeqComID = unsigned int;

	/// 以下为 Visual Sequence Runtime 使用的强类型 ObjectID，底层均为 uint32_t，便于与 JSON/存档对齐。
	using VGSSCharacterObjectID = uint32_t;
	using VGSSSpriteObjectID = uint32_t;
	using VGSSAudioObjectID = uint32_t;
	using VGSSVideoObjectID = uint32_t;
}
