/*
 * StoryExecutionInstance 实现（VGGalgame / ScriptSystem）
 *
 * 中文：构造时持有执行器引用；Tick/Continue 在总线守卫下转发，保证 Sequence 等后端
 * 在协程或回调中能安全访问 SubsystemBus。
 */

#include "ScriptSystem/StoryExecutionInstance.h"

#include "VGGalgameCore/Include/SubsystemBusGuard.h"
#include "NNRuntimeAsset/Interface/Package.h"
#include "VGGalgameCore/Include/Components.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"

namespace VisionGal::GalGame
{
	StoryExecutionInstance::StoryExecutionInstance(const Ref<IStoryScriptExecutor>& executor)
		: Executor(executor)
	{
	}

	void StoryExecutionInstance::Tick(float deltaTime, ISubsystemBus* bus)
	{
		SubsystemBusGuard guard(bus);
		(void)guard;
		if (Executor)
		{
			Executor->Tick(deltaTime);
		}
	}

	void StoryExecutionInstance::Continue(ISubsystemBus* bus)
	{
		SubsystemBusGuard guard(bus);
		(void)guard;
		if (Executor)
		{
			Executor->ContinueDialogue();
		}
	}

	IRuntimeInterface* StoryExecutionInstance::QueryInterface(InterfaceID id)
	{
		if (Executor)
		{
			return Executor->QueryInterface(id);
		}
		return nullptr;
	}
}
