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

#include "SSExecutor.h"
#include "VGGalgameCore/Include/GalGameContext.h"
#include "VGGalgameCore/Include/GalGameEngineAccess.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include "StoryScriptLuaInterface.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <NNCore/Interface/HLog.h>
#include "NNRuntimeVFS/Include/VFSService.h"
//#include "GalGameEngine.h"
#include <sol/state.hpp>

#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "NNPlatformCore/Interface/FileSystem/HFileSystem.h"
#include "NNEngineLegacy/Include/Lua/LuaInterface.h"
#include "LuaBinding.h"
#include "VGLuaCore/LuaErrorManager.h"
#include "VGGalgameCore/Interface/ISceneSubsystem.h"

namespace VisionGal::GalGame
{
    LuaStoryScript::LuaStoryScript()
    {
		GalGame::GalGameLuaBinding::RegisterScript(m_LuaState);
    } 

    Ref<LuaStoryScript> LuaStoryScript::LoadFromFile(const std::string& path)
    {
        Ref<LuaStoryScript> script = MakeRef<LuaStoryScript>();

        script->SetResourcePath(path);

        return script;
    }

    bool LuaStoryScript::Run(ISubsystemBus* bus, IGalGameContext* gameContext)
    {
		m_Bus = bus;
		m_ScriptExecution.Bus = bus;
		m_ScriptExecution.Context = gameContext;
		m_ScriptExecution.HostEngine = GalGameEngineAccess::Current();
		m_Engine = m_ScriptExecution.HostEngine;

		// 记录脚本最后修改时间 
		auto absPath = VFSService::GetInstance()->AbsolutePath(GetResourcePath());
		if (Horizon::HFileSystem::ExistsFile(absPath))
		{
			m_ScriptLastWriteTime = std::filesystem::last_write_time(absPath);
		}

        //m_LuaState["Engine"] = sol::object(m_LuaState, sol::in_place, dynamic_cast<GalGameEngine*>(engine));
        //m_LuaState["引擎"] = sol::object(m_LuaState, sol::in_place, dynamic_cast<GalGameEngine*>(engine));
		m_LuaState["GalGame"]["引擎"] = sol::object(m_LuaState, sol::in_place, m_Engine);
		m_LuaState["VG"] = sol::object(m_LuaState, sol::in_place, m_Engine);

        if (!LoadScript(GetResourcePath()))
            return false;

        StoryScriptLuaInterface::SetStoryScriptCoroutine(&m_Coroutine);
		StoryScriptLuaInterface::SetCurrentStoryScriptPath(GetResourcePath());

        PreLoadScriptResource();

    	auto result = m_Coroutine();
    
        if (m_Coroutine.error())
        {
            sol::error err = result;
            std::string errorStr = err.what();
            H_LOG_ERROR(err.what());

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = GetResourcePath();
			evt.ErrorMessage = err.what();
			evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);

            return false;
        }

        return true;
    }

    void LuaStoryScript::Tick(float deltaTime)
    {
    }

    IRuntimeInterface* LuaStoryScript::QueryInterface(InterfaceID id)
    {
		return nullptr;
    }

    void LuaStoryScript::PreLoadScriptResource()
    {
		if (m_Bus == nullptr)
			return;

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
			if (m_Bus != nullptr)
				m_Bus->Scene()->PreLoadResource(path);
        }
    }

    void LuaStoryScript::ContinueDialogue()
    {
		StoryScriptLuaInterface::Continue();
    }

    void LuaStoryScript::OnChoiceSelected(const std::string& id, const std::vector<std::string>& options,
	    int currentChoice)
    {
		StoryScriptLuaInterface::Continue(StoryScriptLuaInterface::ContinueType::String, 0, options[currentChoice]);
    }

    void LuaStoryScript::OnInputSubmitted(const std::string& id, const std::string& text)
    {
		StoryScriptLuaInterface::Continue(StoryScriptLuaInterface::ContinueType::String, 0, text);
    }

    bool LuaStoryScript::LoadScript(const String& file)
    {
		auto result = VFSService::ReadTextFromFile(file, m_ScriptCode);

		if (!result)
		{
			H_LOG_ERROR("Failed to read story script file: %s", file.c_str());
			return false;
		}

        try {
            m_Coroutine = m_LuaState.script(m_ScriptCode);

			//auto result = m_LuaState.safe_script(m_ScriptCode, sol::script_pass_on_error);
			//if (result.valid()) {
			//	m_Coroutine = result;
			//}
			//else {
			//	sol::error err = result;
			//
			//	H_LOG_ERROR("%s Error: %s", GetResourcePath().c_str(), err.what());
			//
			//	// 事件
			//	LuaScriptEvent evt;
			//	evt.EventType = LuaScriptEventType::ScriptError;
			//	evt.ScriptPath = file;
			//	evt.ErrorMessage = err.what();
			//	evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(err.what());
			//	EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
			//	return false;
			//}
        }
        catch (const sol::error& e) {
            H_LOG_ERROR(e.what());
			//std::cout << "Lua Script Load Error: " << VGLuaCoreGetErrorLineNumber() << std::endl;

			// 事件
			LuaScriptEvent evt;
			evt.EventType = LuaScriptEventType::ScriptError;
			evt.ScriptPath = file;
			evt.ErrorMessage = e.what();
			evt.ErrorLineNumber = VGLuaInterface::ExtractErrorLineNumber(e.what());
			EngineEventBus::Get().OnLuaScriptEvent.Invoke(evt);
            return false;
        }

        return true;
    }
}
