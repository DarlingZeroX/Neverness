/*
 * IRuntimeEventPipeline — 运行时消息管线（Phase 8 Contract 骨架）
 *
 * 中文：目标为替代分散的 GalEngineEventBus / GalGameUIEventBus「纯广播」模型，支持 Capture / Replay / Filter；
 * Phase 8 先引入接口与空实现挂载点，具体管线接入在后续子阶段完成。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IRuntimeEventPipeline
	{
		virtual ~IRuntimeEventPipeline() = default;
	};
}
