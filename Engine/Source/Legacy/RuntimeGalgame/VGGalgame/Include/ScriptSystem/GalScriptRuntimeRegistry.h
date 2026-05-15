/*
 * GalScriptRuntimeRegistry — 脚本运行时注册表（Phase 8B / VGGalgame）
 *
 * 中文：**StoryScriptSystem** 在解析路径时优先遍历本表（**CanLoad** 首个命中），
 * 未命中时再回退 **GalGameScriptExecutorFactory**，保证旧资产与未注册后端的兼容性。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include "VGGalgameContract/Interface/IScriptRuntime.h"
#include <vector>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API GalScriptRuntimeRegistry final
	{
	public:
		GalScriptRuntimeRegistry() = default;

		/// 中文：后注册者优先匹配（便于测试覆盖内置）；生产路径应固定注册顺序。
		void RegisterRuntime(Ref<IScriptRuntime> runtime);

		/// 中文：清空全部运行时（供测试或将来热重载注册表重置）。
		void Clear() noexcept;

		/// 中文：返回第一个 **CanLoad(path)==true** 的运行时；若无则 **nullptr**。
		IScriptRuntime* FindRuntimeForPath(const String& path) const noexcept;

	private:
		std::vector<Ref<IScriptRuntime>> m_Runtimes;
	};
}
