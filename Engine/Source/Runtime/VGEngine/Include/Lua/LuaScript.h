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
#include "../EngineConfig.h"
#include "VGCore/Interface/GameInterface.h"
#include <sol/table.hpp>
#include <sol/function.hpp>
#include <sol/state.hpp>

namespace VisionGal
{
	class VG_ENGINE_API LuaScript: public IScript
	{
	public:
		using ScriptVariableTable = std::unordered_map<String, IScriptVariable>;

		LuaScript() = default;
		LuaScript(const LuaScript&) = default;
		LuaScript& operator=(const LuaScript&) = default;
		LuaScript(LuaScript&&) noexcept = default;
		LuaScript& operator=(LuaScript&&) noexcept = default;
		~LuaScript() override = default;

		static Ref<LuaScript> LoadFromFile(const String& file);
		String GetScriptType() override;
		
		void Awake(GameActor* actor) override;
		void Start(GameActor* actor) override;
		void Update(GameActor* actor, float deltaTime) override;
		void FixedUpdate(GameActor* actor) override;

		ScriptVariableTable& GetVariables() override { return m_Variables; };
		void SetVariable(IScriptVariable& variable);
		void SetVariable(const ScriptVariableTable& variables);

		void Initialize();
		Ref<LuaScript> Reload(); 
	private:
		void ReadScriptVariables();
	private:
		String m_ScriptPath;
		std::string m_ScriptString;
		sol::state m_LuaState;
		sol::table m_Script;

		sol::function m_UpdateFunction; // Lua 更新函数
		sol::function m_StartFunction;

		ScriptVariableTable  m_Variables;
		bool m_IsError;
	};


}
