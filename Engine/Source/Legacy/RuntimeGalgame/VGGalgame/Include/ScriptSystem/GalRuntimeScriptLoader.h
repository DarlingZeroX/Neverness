/*
 * GalRuntimeScriptLoader — 剧情脚本「路径 → 执行器」加载编排（Phase 8B）
 *
 * 中文：从 **StoryScriptSystem** 抽出「**GalScriptRuntimeRegistry** 优先 → **GalGameScriptExecutorFactory** 回退」，
 * 使宿主类只负责 **Run** / **Tick** / UI 桥接，降低 **God Object** 倾向。线程：仅游戏线程调用。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include "GalScriptRuntimeRegistry.h"
#include "VGGalgameCore/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API GalRuntimeScriptLoader final
	{
	public:
		/// 中文：在 **StoryScriptSystem::Initialise** 内绑定；允许 **nullptr**（此时 **LoadExecutorForPath** 仅走工厂）。
		void Attach(GalScriptRuntimeRegistry* registry) noexcept { m_Registry = registry; }

		/// 中文：**Registry.CreateScriptExecutor** 成功则返回；否则 **GetAssetTypeNameID + GalGameScriptExecutorFactory**。
		Ref<IStoryScriptExecutor> LoadExecutorForPath(const String& path);

	private:
		GalScriptRuntimeRegistry* m_Registry = nullptr;
	};
}
