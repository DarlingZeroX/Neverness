/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * 中文说明：
 * - 本类型是 **`IStoryExecutionInstance`** 的薄包装实现，由 **`StoryScriptSystem`** 在成功
 *   `LoadStoryScript` 后与具体 **`IStoryScriptExecutor`** 成对持有。
 * - **`Tick` / `Continue`**：在设置当前 **`ISubsystemBus*`**（`SubsystemBusGuard`）后转发到
 *   具体执行器；Lua 后端可忽略总线，Sequence 后端需总线以访问 Scene/Dialogue 等子系统。
 * - **`QueryInterface`**：透传到底层执行器，便于上层按具体类型（如 Sequence 内核）查询扩展接口。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include "VGGalgameCore/Interface/IStoryScriptExecutor.h"
#include <VGGalgameCore/Interface/IStoryScriptSystem.h>

namespace VisionGal::GalGame
{
	struct VG_GALGAME_API StoryExecutionInstance : public IStoryExecutionInstance
	{
		StoryExecutionInstance(const Ref<IStoryScriptExecutor>& executor);
		~StoryExecutionInstance() override = default;

		void Tick(float deltaTime, ISubsystemBus* bus) override;
		void Continue(ISubsystemBus* bus) override;
		IRuntimeInterface* QueryInterface(InterfaceID id) override;

		template<typename T>
		T* ExecutionQuery() {
			return static_cast<T*>(QueryInterface(typeid(T)));
		}
	private:
		Ref<IStoryScriptExecutor> Executor = nullptr;
	};

}
