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
#include "VGGalgameCore/Include/SubsystemBusGuard.h"
#include "VGGalgameCore/Interface/IGalGameContext.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGCore/Include/Core/VFS.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Core/EventBus.h"
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

    bool SSExecutorSequence::Run(ISubsystemBus* bus, IGalGameContext* gameContext)
    {
        (void)gameContext;
        PreLoadScriptResource();

		m_ExecutionContext.SubsystemBus = bus;
		// ResourceManager 默认构造；SubsystemBus 由宿主在 Run 时注入执行上下文。
		m_ExecutionContext.ResourceManager = MakeRef<SSExecutorResourceManager>();
		m_ExecutionContext.SequenceData = m_ExecutionData->SequenceData;

		const Ref<SequenceExecutionInstance> kernel = MakeRef<SequenceExecutionInstance>();
		kernel->SetExecutionContext(&m_ExecutionContext);
		kernel->Play();
		m_Executor = kernel;

        return true;
    }

    void SSExecutorSequence::Tick(float deltaTime)
    {
		SubsystemBusGuard guard(m_ExecutionContext.SubsystemBus);
		if (!guard)
			return;
		if (m_Executor != nullptr)
		{
			m_Executor->Tick(deltaTime, guard.Get());
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
		SubsystemBusGuard guard(m_ExecutionContext.SubsystemBus);
		if (!guard)
			return;
		if (m_Executor != nullptr)
		{
			m_Executor->Continue(guard.Get());
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
