/*
 * IGalGameContext — Gal 运行时聚合上下文抽象（SubsystemBus 重构配套）
 *
 * 具体实现为 GalGameContext；引擎瘦身后通过 IGalGameEngine::GetContext() 暴露，
 * 避免脚本与序列层直接依赖具体上下文类型。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct  IGalGameContext
	{
		virtual ~IGalGameContext() = default;
	};
}
