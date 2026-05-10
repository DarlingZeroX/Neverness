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

#pragma once
#include "../VGGalgameRuntimeConfig.h"
#include "IStoryScript.h"

namespace VisionGal::GalGame
{
	struct StoryExecutionInstance : public IStoryExecutionInstance
	{
		StoryExecutionInstance(const Ref<IStoryScriptExecutor>& executor);
		~StoryExecutionInstance() override = default;

		void Tick(float deltaTime) override;
		void Continue() override;
		IRuntimeInterface* QueryInterface(InterfaceID id) override;

		template<typename T>
		T* ExecutionQuery() {
			return static_cast<T*>(QueryInterface(typeid(T)));
		}
	private:
		Ref<IStoryScriptExecutor> Executor = nullptr;		
	};

}
