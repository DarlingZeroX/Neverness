/*
 * IScriptSubsystem — 剧情脚本加载 / 等待 / 执行器访问（Phase 8：Contract）
 *
 * 对齐原 IGalGameEngine 上脚本相关入口；Tick 由 StoryScriptSystem 仍在引擎 Update 中驱动。
 */

#pragma once
#include "IStoryScriptSystem.h"
#include "../VGGalCoreConfig.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	struct IScriptSubsystem
	{
		virtual ~IScriptSubsystem() = default;

		virtual bool LoadStoryScript(const String& path) = 0;
		virtual void LoadStoryScriptOnUpdate(const String& path) = 0;
		virtual void ReloadStoryScript() = 0;
		virtual void Wait(float duration) = 0;
		virtual IStoryScriptSystem* GetStoryScriptSystem() = 0;
	};
}
