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

#include <regex>
#include <sol/state.hpp>
#include <HCore/Interface/HLog.h>
#include "SSExecutor.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"
#include "HFileSystem/Interface/HFileSystem.h"

namespace VisionGal::GalGame
{
	SSExecutorVisual::SSExecutorVisual()
    {
		//GalGame::GalGameLuaBinding::RegisterScript(m_LuaState);
    } 

    Ref<SSExecutorVisual> SSExecutorVisual::LoadFromFile(const std::string& path)
    {
        Ref<SSExecutorVisual> script = MakeRef<SSExecutorVisual>();

        script->SetResourcePath(path);

        return script;
    }

    bool SSExecutorVisual::Run(IGalGameEngine* engine)
    {
		// 记录脚本最后修改时间 
		auto absPath = VFS::GetInstance()->AbsolutePath(GetResourcePath());
		if (Horizon::HFileSystem::ExistsFile(absPath))
		{
			m_ScriptLastWriteTime = std::filesystem::last_write_time(absPath);
		}

        if (!LoadScript(GetResourcePath()))
            return false;

        PreLoadScriptResource();

        return true;
    }

    void SSExecutorVisual::PreLoadScriptResource()
    {

    }

    void SSExecutorVisual::ContinueDialogue()
    {

    }

    void SSExecutorVisual::OnChoiceSelected(const std::string& id, const std::vector<std::string>& options,
	    int currentChoice)
    {

    }

    void SSExecutorVisual::OnInputSubmitted(const std::string& id, const std::string& text)
    {

    }

    bool SSExecutorVisual::LoadScript(const String& file)
    {
		auto result = VFS::ReadTextFromFile(file, m_ScriptCode);

		if (!result)
		{
			H_LOG_ERROR("Failed to read story script file: %s", file.c_str());
			return false;
		}

        try {
        }
        catch (const sol::error& e) {
            return false;
        }

        return true;
    }
}
