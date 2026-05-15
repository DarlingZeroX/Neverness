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

#include "../GSSExport.h"
#include "VGCore/Include/Core/Core.h"
#include "VGGalgameCore/Interface/IGalGameContext.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGGalgameCore/Interface/IStoryScriptExecutor.h"
#include "VGGalgameCore/Interface/IRuntimeExecutionServices.h"

#include "Runtime/IStoryExecutionInstance.h"
#include "Runtime/SequenceExecutionInstance.h"
#include "SequenceExecutionData.h"
#include <memory>
#include "../Interface/IVGSSequenceComponent.h"

namespace VisionGal::GalGame
{
	class VG_GSS_API SSExecutorSequence : public IStoryScriptExecutor
	{ 
	public:
		SSExecutorSequence();
		~SSExecutorSequence() override = default;

		static Ref<SSExecutorSequence> LoadFromAsset(const String& file);

		bool Run(ISubsystemBus* bus, IGalGameContext* gameContext) override;
		void Tick(float deltaTime) override;
		IRuntimeInterface* QueryInterface(InterfaceID id) override;

		void ContinueDialogue() override;
		void OnChoiceSelected(const std::string& id,const std::vector<std::string>& options,int currentChoice) override; 
		void OnInputSubmitted(const std::string& id, const std::string& text) override; 

		void PreLoadScriptResource() override;
		std::filesystem::file_time_type GetScriptLastWriteTime() const override { return m_ScriptLastWriteTime; }
	private:
		bool LoadScript(const String& file);
	private:
		std::filesystem::file_time_type m_ScriptLastWriteTime;

		Ref<SSSequenceExecutionData> m_ExecutionData;
		SSSequenceExecutionContext m_ExecutionContext;
		/// Sequence Runtime Kernel（`SequenceExecutionInstance`），以接口指针持有便于测试替换实现。
		Ref<IStoryExecutionInstance> m_Executor = nullptr;
		std::unique_ptr<IRuntimeExecutionServices> m_RuntimeExecutionServices;
	};
}
