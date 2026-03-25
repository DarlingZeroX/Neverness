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
#include "../Interface/IGalGameEngine.h"
#include "VGCore/Include/Core/Core.h"
#include <sol/state.hpp>
#include <sol/coroutine.hpp>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API LuaStoryScript : public VGEngineResource
	{ 
	public:
		LuaStoryScript();
		~LuaStoryScript() override = default;

		static Ref<LuaStoryScript> LoadFromFile(const String& file);
		bool Run(IGalGameEngine* engine);

		sol::coroutine GetCoroutine() { return m_Coroutine; }
		void PreLoadScriptResource();

		std::filesystem::file_time_type GetScriptLastWriteTime() const { return m_ScriptLastWriteTime; }
	private:
		bool LoadScript(const String& file);
	private:
		String m_ScriptCode;
		sol::state m_LuaState;
		sol::coroutine m_Coroutine;
		std::filesystem::file_time_type m_ScriptLastWriteTime;
	};
}