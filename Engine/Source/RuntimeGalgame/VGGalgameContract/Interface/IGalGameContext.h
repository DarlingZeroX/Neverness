/*
 * IGalGameContext — Gal 运行时聚合上下文抽象（Phase 8：Contract 纯虚）
 *
 * 具体实现为 VGGalgameRuntimeCore 中的 GalGameContext；引擎通过 IGalGameEngine::GetContext() 暴露，
 * 避免脚本与序列层直接依赖具体上下文类型。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IGalGameContext
	{
		virtual ~IGalGameContext() = default;
	};
}
