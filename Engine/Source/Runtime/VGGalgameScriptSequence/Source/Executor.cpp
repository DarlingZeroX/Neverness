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
#include "Executor.h"
#include "Asset/Asset.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"
#include "HFileSystem/Interface/HFileSystem.h"

namespace VisionGal::GalGame
{
	SSExecutorSequence::SSExecutorSequence()
    {
		//GalGame::GalGameLuaBinding::RegisterScript(m_LuaState);
    } 

    Ref<SSExecutorSequence> SSExecutorSequence::LoadFromAsset(const std::string& path)
    {
        Ref<SSExecutorSequence> script = MakeRef<SSExecutorSequence>();
        script->SetResourcePath(path);
		if (script->LoadScript(path))
			return script;
        return nullptr;
    }

    bool SSExecutorSequence::Run(IGalGameEngine* engine)
    {
        PreLoadScriptResource();

		m_ExecutionContext.Engine = engine;
		// SSExecutorResourceManager 仅有默认构造；引擎指针写在 ExecutionContext.Engine。
		m_ExecutionContext.ResourceManager = MakeRef<SSExecutorResourceManager>();
		m_ExecutionContext.SequenceData = m_ExecutionData->SequenceData;

		m_Executor = MakeRef<SSSequenceExecutor>();
		m_Executor->SetExecutionContext(&m_ExecutionContext);
		m_Executor->Play();

        return true;
    }

    void SSExecutorSequence::Tick(float deltaTime)
    {
		if (m_Executor != nullptr)
		{
			m_Executor->Tick(deltaTime);
		}
    }

    IRuntimeInterface* SSExecutorSequence::QueryInterface(InterfaceID id)
    {
		if (m_Executor != nullptr)
		{
			return m_Executor->QueryInterface(id);
		}

		return nullptr;
    }

    void SSExecutorSequence::PreLoadScriptResource()
    {

    }

    void SSExecutorSequence::ContinueDialogue()
    {
		if (m_Executor != nullptr)
		{
			m_Executor->Continue();
		}
    }

    void SSExecutorSequence::OnChoiceSelected(const std::string& id, const std::vector<std::string>& options,
	    int currentChoice)
    {

    }

    void SSExecutorSequence::OnInputSubmitted(const std::string& id, const std::string& text)
    {

    }

    bool SSExecutorSequence::LoadScript(const String& path)
    {
		// 记录脚本最后修改时间 
		auto absPath = VFS::GetInstance()->AbsolutePath(GetResourcePath());
		if (Horizon::HFileSystem::ExistsFile(absPath))
		{
			m_ScriptLastWriteTime = std::filesystem::last_write_time(absPath);
		}

		GalGame::SequenceScriptAssetLoader loader;
		Ref<VGAsset> asset = nullptr;
		if (loader.Read(path, asset) == false)
			return false;

		if (asset == nullptr)
			return false;

		Ref<GalGame::SequenceScriptAsset> scriptAsset = std::dynamic_pointer_cast<GalGame::SequenceScriptAsset>(asset);
		if (scriptAsset == nullptr)
			return false;

		m_ExecutionData = scriptAsset->ExecutionData;

        return true;
    }
}
