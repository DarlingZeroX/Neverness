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

#include "Galgame/StoryScript.h"
#include "Core/Core.h"
#include "Galgame/StoryScriptLuaInterface.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <HCore/Include/Core/HLog.h>
#include "Core/VFS.h"
#include "Galgame/GalGameEngine.h"
#include "Galgame/GameEngineCore.h"
#include <sol/state.hpp>

#include "Core/EventBus.h"
#include "VGLuaCore/LuaErrorManager.h"

namespace VisionGal::GalGame
{
    LuaStoryScript::LuaStoryScript()
    {
        StoryScriptLuaInterface::Initialise(m_LuaState);
    } 

    Ref<LuaStoryScript> LuaStoryScript::LoadFromFile(const std::string& file)
    {
        Ref<LuaStoryScript> script = CreateRef<LuaStoryScript>();

        script->SetResourcePath(file);

        return script;
    }

    bool LuaStoryScript::Run(IGalGameEngine* engine)
    {
        //m_LuaState["Engine"] = sol::object(m_LuaState, sol::in_place, dynamic_cast<GalGameEngine*>(engine));
        //m_LuaState["引擎"] = sol::object(m_LuaState, sol::in_place, dynamic_cast<GalGameEngine*>(engine));
		m_LuaState["GalGame"]["引擎"] = sol::object(m_LuaState, sol::in_place, dynamic_cast<GalGameEngine*>(engine));

        if (!LoadScript(GetResourcePath()))
            return false;

        StoryScriptLuaInterface::SetStoryScriptCoroutine(&m_Coroutine);

        PreLoadScriptResource();

    	auto result = m_Coroutine();
    
        if (m_Coroutine.error())
        {
            sol::error err = result;
            std::string errorStr = err.what();
            H_LOG_ERROR(err.what());
            return false;
        }

        return true;
    }

    void LuaStoryScript::PreLoadScriptResource()
    {
        std::istringstream inputStream(m_ScriptCode);
        std::string line;

        std::set<std::string> resourcePaths;

        std::regex resourceRegex(R"(['"]([^'"]+\.(png|jpg|jpeg|tga|bmp|wav|mp3|ogg))['"])");
        std::smatch match;

        // 根据正则表达式识别资源
        while (std::getline(inputStream, line)) {
            auto begin = line.cbegin();
            auto end = line.cend();

            while (std::regex_search(begin, end, match, resourceRegex)) {
                resourcePaths.insert(match[1].str());
                begin = match.suffix().first;
            }
        }

        // 预加载资源
        for (const auto& path : resourcePaths) {
            GameEngineCore::GetCurrentEngine()->PreLoadResource(path);
        }
    }

    bool LuaStoryScript::LoadScript(const String& file)
    {
		auto result = VFS::ReadTextFromFile(file, m_ScriptCode);

		if (!result)
		{
			H_LOG_ERROR("Failed to read story script file: %s", file.c_str());
			return false;
		}

        try {
            m_Coroutine = m_LuaState.script(m_ScriptCode);
			//sol::protected_function_result result = m_LuaState.safe_script(m_ScriptCode, sol::script_pass_on_error);
			//
			//if (!result.valid()) {
			//	sol::error err = result;
			//
			//	// 获取调用栈
			//	sol::function debug_traceback = m_LuaState["debug"]["traceback"];
			//	if (debug_traceback.valid()) {
			//		std::string stack_trace = debug_traceback();
			//		H_LOG_ERROR("Lua错误: {}\n调用栈: {}", err.what(), stack_trace);
			//	} else {
			//		H_LOG_ERROR("Lua错误: {}", err.what());
			//	}
			//	return false;
			//}

			//m_Coroutine = result;
        }
        catch (const sol::error& e) {
            H_LOG_ERROR(e.what());
			std::cout << "Lua Script Load Error: " << VGLuaCoreGetErrorLineNumber() << std::endl;

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = file;
			evt.ErrorMessage = e.what();
			evt.ErrorLineNumber = VGLuaCoreGetErrorLineNumber();
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
            return false;
        }

        return true;
    }
}
