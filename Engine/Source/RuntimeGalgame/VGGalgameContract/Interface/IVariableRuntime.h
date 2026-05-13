/*
 * IVariableRuntime — Gal 变量运行时契约（Phase 8 占位）
 *
 * 中文：未来由独立 VariableRuntime 实现脚本变量 / 标志位；当前宿主可固定返回 nullptr。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IVariableRuntime
	{
		virtual ~IVariableRuntime() = default;
	};
}
