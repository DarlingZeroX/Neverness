/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "StoryExecutionInstance.h"

#include "VGAsset/Interface/Package.h"
#include "VGGalgameCore/Include/Components.h"
#include "VGCore/Include/Core/EventBus.h"

namespace VisionGal::GalGame
{
	StoryExecutionInstance::StoryExecutionInstance(const Ref<IStoryScriptExecutor>& executor)
		: Executor(executor)
	{
	}

	void StoryExecutionInstance::Tick(float deltaTime)
	{
		if (Executor)
		{
			Executor->Tick(deltaTime);
		}
	}

	void StoryExecutionInstance::Continue()
	{
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
