#pragma once
#include "../Interface/GameInterface.h"
#include "../Lua/sol2/sol.hpp"

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
