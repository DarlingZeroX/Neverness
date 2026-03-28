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
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	struct  IStoryScriptExecutor : public VGEngineResource
	{ 
		 ~IStoryScriptExecutor() override = default;

		virtual bool Run(IGalGameEngine* engine) = 0;

		virtual void PreLoadScriptResource() = 0;

		virtual std::filesystem::file_time_type GetScriptLastWriteTime() const = 0;

		virtual void ContinueDialogue() = 0;

		virtual void OnChoiceSelected(const std::string& id,const std::vector<std::string>& options,int currentChoice) = 0; 
		virtual void OnInputSubmitted(const std::string& id, const std::string& text) = 0; 
	};

	struct IStoryScriptExecutorCreator
	{
		virtual ~IStoryScriptExecutorCreator() = default;

		virtual Ref<IStoryScriptExecutor> LoadFromFile(const String& path) = 0;
	};

	struct VG_GALGAME_RUNTIME_API StoryScriptExecutorFactory
	{
		virtual Ref<IStoryScriptExecutor> LoadStoryScriptExecutorFromFile(const String& type, const String& path);

		virtual void RegisterStoryScriptExecutorType(const String& type, Ref<IStoryScriptExecutorCreator> factoryFunction);

		static StoryScriptExecutorFactory& GetInstance();

	private:
		std::unordered_map<String, Ref<IStoryScriptExecutorCreator>> m_FactoryFunctions;
	};

}
