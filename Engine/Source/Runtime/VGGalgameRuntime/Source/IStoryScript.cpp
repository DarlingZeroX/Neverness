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
#include "IStoryScript.h"

namespace VisionGal::GalGame
{
	Ref<IStoryScriptExecutor> StoryScriptExecutorFactory::LoadStoryScriptExecutorFromFile(const String& type, const String& path)
	{
		if (path.empty())
		{
			return nullptr;
		}

		if (m_FactoryFunctions.find(type) == m_FactoryFunctions.end())
		{
			H_LOG_WARN("尝试加载未注册类型的剧情脚本执行器: %s", type.c_str());

			return nullptr;
		}

		return m_FactoryFunctions[type]->LoadFromFile(path);
	}

	void StoryScriptExecutorFactory::RegisterStoryScriptExecutorType(
		const String& type,
		Ref<IStoryScriptExecutorCreator> factoryFunction
	)
	{
		m_FactoryFunctions[type] = factoryFunction;
	}


	StoryScriptExecutorFactory& StoryScriptExecutorFactory::GetInstance()
	{
		static StoryScriptExecutorFactory instance;
		return instance;
	}

}