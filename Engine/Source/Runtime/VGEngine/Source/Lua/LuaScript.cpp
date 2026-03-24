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

#include "Lua/LuaScript.h"
#include "VGCore/Include/Core/VFS.h"
#include "Lua/LuaInterface.h"
#include <sol/sol.hpp>

#include "Scene/GameActor.h"
#include "VGCore/Include/Core/EventBus.h"

namespace VisionGal
{
	Ref<LuaScript> LuaScript::LoadFromFile(const String& file)
	{
		auto script = MakeRef<LuaScript>();

		auto result = VFS::ReadTextFromFile(file, script->m_ScriptString);
		 
		if (!result)
			return nullptr;

		script->SetResourcePath(file);
		script->m_ScriptPath = file;
		script->Initialize();

		return script;
	}

	void LuaScript::Initialize()
	{
		m_LuaState.open_libraries(sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table); // 默认已加载这些库
		VGLuaInterface::Initialise(m_LuaState);

		m_IsError = false;

		try {
			auto result = m_LuaState.safe_script(m_ScriptString, sol::script_pass_on_error);
			if (result.valid()) {
				m_Script = result;
			}
			else {
				sol::error err = result;
				H_LOG_ERROR("%s Error: %s", m_ScriptPath.c_str(), err.what());
				m_IsError = true;

				// 事件
				LuaScriptEvent evt;
				evt.EventType = LuaScriptEventType::ScriptError;
				evt.ScriptPath = GetResourcePath();
				evt.ErrorMessage = err.what();
				evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
				EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
			}
		}
		catch (const sol::error& err) {
			m_IsError = true;
			H_LOG_ERROR("%s Error: %s", m_ScriptPath.c_str(), err.what());

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = GetResourcePath();
			evt.ErrorMessage = err.what();
			evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
		}

		if (m_IsError == false)
		{
			m_StartFunction = m_Script["Start"];
			m_UpdateFunction = m_Script["Update"];

			ReadScriptVariables();
		}
	}

	Ref<LuaScript> LuaScript::Reload()
	{
		return LoadFromFile(this->m_ScriptPath);
	}

	void LuaScript::ReadScriptVariables()
	{
		// 遍历并存储变量
		m_Script.for_each([&](sol::object key, sol::object value) {
			std::string name = key.as<std::string>();
			if (name == "_G" || name == "_VERSION")// 排除特定内置变量
				return; 
			if (value.get_type() == sol::type::function || value.get_type() == sol::type::thread)
				return;

			IScriptVariable var;
			var.VariableName = name;
			var.NativeType = int(value.get_type());

			switch (value.get_type())
			{
			case sol::type::string:
				var.VariableType = "String";
				var.ValueString = value.as<std::string>();
				break;
			case sol::type::number:
				var.VariableType = "Number";
				var.ValueDouble = value.as<double>();
				break;
			case sol::type::boolean:
				var.VariableType = "Boolean";
				var.ValueDouble = value.as<bool>();
				break;
			default: break;
			}

			m_Variables[name] = var;
			
		});
	}

	String LuaScript::GetScriptType()
	{
		return "LuaScript";
	}

	void LuaScript::Awake(IGameActor* actor)
	{
		if (m_IsError)
			return;

		for (auto& var : m_Variables)
		{
			auto& v = var.second;
			if (v.VariableType == "Number")
			{
				m_Script[v.VariableName] = sol::object(m_LuaState, sol::in_place, v.ValueDouble);;
			}
			else if (v.VariableType == "String")
			{
				m_Script[v.VariableName] = sol::object(m_LuaState, sol::in_place, v.ValueString);;
			}
			else if (v.VariableType == "Boolean")
			{
				m_Script[v.VariableName] = sol::object(m_LuaState, sol::in_place, v.ValueBoolean);;
			}
			else if (v.VariableType == "GameActor")
			{
				auto* scene = dynamic_cast<IScene*>(actor->GetScene());
				auto* actor = dynamic_cast<GameActor*>(scene->GetActor(v.ValueEntityID));
				if (actor)
				{
					m_Script[v.VariableName] = sol::object(m_LuaState, sol::in_place, actor);
				}
			}
		}

		GameActor* ga = dynamic_cast<GameActor*>(actor);
		H_ASSERT_NOT_NULL(ga);
		m_Script["gameObject"] = sol::object(m_LuaState, sol::in_place, ga);
	}

	void LuaScript::Start(IGameActor* actor)
	{
		// 检查函数是否有效
		if (!m_StartFunction.valid()) {
			H_LOG_ERROR("LuaScript::Start: m_StartFunction is not valid");
			return;
		}

		if (m_StartFunction)
		{
			//m_StartFunction(actor);
			// 使用安全调用的方式
			sol::protected_function_result result = m_StartFunction(m_Script);
			if (!result.valid()) {
				sol::error err = result;
				H_LOG_ERROR("%s Error: %s", m_ScriptPath.c_str(), err.what());

				// 事件
				LuaScriptEvent evt;
				evt.EventType = LuaScriptEventType::ScriptError;
				evt.ScriptPath = GetResourcePath();
				evt.ErrorMessage = err.what();
				evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
				EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
			}
		}
	}

	void LuaScript::Update(IGameActor* actor, float deltaTime)
	{
		// 检查函数是否有效
		if (!m_UpdateFunction.valid()) {
			H_LOG_ERROR("LuaScript::Start: m_UpdateFunction is not valid");
			return;
		}

		if (m_StartFunction)
		{
			//m_StartFunction(actor);
			// 使用安全调用的方式
			sol::protected_function_result result = m_UpdateFunction(m_Script, deltaTime);
			if (!result.valid()) {
				sol::error err = result;
				H_LOG_ERROR("%s Error: %s", m_ScriptPath.c_str(), err.what());

				// 事件
				LuaScriptEvent evt;
				evt.EventType = LuaScriptEventType::ScriptError;
				evt.ScriptPath = GetResourcePath();
				evt.ErrorMessage = err.what();
				evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
				EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
			}
		}
	}

	void LuaScript::FixedUpdate(IGameActor* actor)
	{

	}

	void LuaScript::SetVariable(IScriptVariable& variable)
	{

	}

	void LuaScript::SetVariable(const ScriptVariableTable& variables)
	{
		m_Variables = variables;
	}
}
