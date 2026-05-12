/*
 * IUISubsystem — Gal UI 门面（选择 / 全屏字 / 输入等，由 ISubsystemBus::UI() 访问）
 *
 * 当前阶段委托既有 IGalGameUISystem；后续可再细分接口而不动总线形状。
 */

#pragma once
#include "IGameSystem.h"
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct  IUISubsystem
	{
		virtual ~IUISubsystem() = default;

		virtual IGalGameUISystem* GetGalGameUISystem() = 0;
	};
}
