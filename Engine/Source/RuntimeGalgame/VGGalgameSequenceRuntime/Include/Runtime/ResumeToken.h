/*
 * ResumeToken — 与 WaitToken 配对的纯 ID 恢复句柄（用于 Continue(ResumeToken) 精确唤醒）
 */
#pragma once

#include <cstdint>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	struct VG_GSS_API ResumeToken
	{
		std::uint64_t TokenID = 0;
	};
}
