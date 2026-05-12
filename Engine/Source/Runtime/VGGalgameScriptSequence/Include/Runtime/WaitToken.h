/*
 * WaitToken — 异步闸门创建时返回的句柄（仅 ID + 调试原因字符串）
 */
#pragma once

#include <cstdint>
#include <string>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	struct VG_GSS_API WaitToken
	{
		std::uint64_t TokenID = 0;
		std::string Reason;
	};
}
