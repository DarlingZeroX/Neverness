/*
 * IPlaybackSubsystem — 剧情节拍 / Wait 等「播放控制」契约（Phase 8）
 *
 * 中文：从 IGalGameEngine 与 IScriptSubsystem 中抽出与「时间等待、全局节拍」相关的入口，
 * 避免脚本子系统同时承担调度语义；实现通常委托 StoryScriptSystem::Wait。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IPlaybackSubsystem
	{
		virtual ~IPlaybackSubsystem() = default;

		/// 中文：协程式等待（秒）；与历史 `IGalGameEngine::Wait` 行为一致。
		virtual void Wait(float durationSeconds) = 0;
	};
}
