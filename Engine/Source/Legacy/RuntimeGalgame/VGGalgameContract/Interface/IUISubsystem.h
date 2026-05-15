/*
 * IUISubsystem — Gal UI 门面（Phase 8：Contract）
 *
 * 当前阶段委托既有 IGalGameUISystem；后续可再细分接口而不动总线形状。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IGalGameUISystem;

	struct IUISubsystem
	{
		virtual ~IUISubsystem() = default;

		virtual IGalGameUISystem* GetGalGameUISystem() = 0;
	};
}
