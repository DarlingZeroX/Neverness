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
#include "../VGGalScriptVisualConfig.h"
#include "VGCore/Include/Core/Core.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "VGGalgameRuntime/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_VISUAL_SCRIPT_API SSExecutorVisual : public IStoryScriptExecutor
	{ 
	public:
		SSExecutorVisual();
		~SSExecutorVisual() override = default;

		static Ref<SSExecutorVisual> LoadFromFile(const String& file);

		bool Run(IGalGameEngine* engine) override;
		void ContinueDialogue() override;
		void OnChoiceSelected(const std::string& id,const std::vector<std::string>& options,int currentChoice) override; 
		void OnInputSubmitted(const std::string& id, const std::string& text) override; 

		void PreLoadScriptResource() override;
		std::filesystem::file_time_type GetScriptLastWriteTime() const override { return m_ScriptLastWriteTime; }


	private:
		bool LoadScript(const String& file);
	private:
		String m_ScriptCode;
		//sol::state m_LuaState;
		std::filesystem::file_time_type m_ScriptLastWriteTime;
	};
}
