/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Runtime/SequenceExecutionController.h"

#include "HCore/Interface/HLog.h"
#include "VGEngine/Include/Engine/Manager/SceneManager.h"
#include "VGGalgameCore/Interface/GameEngineCore.h"
#include "VGGalgameScriptSequence/Include/SequenceRuntimeTypes.h"

namespace VisionGal::Editor
{
	bool SequenceExecutionController::ExecuteTo(const std::string& assetPath, unsigned int targetEntryIndex, SequenceRuntimeSnapshot& out)
	{
		out = SequenceRuntimeSnapshot{};
		out.CurrentIndex = 0;
		out.HasValidDebugInfo = false;
		out.ReachedTarget = false;
		out.LastError.clear();

		if (assetPath.empty())
		{
			out.LastError = "empty asset path";
			return false;
		}

		if (!SceneManager::Get()->IsPlayMode())
			SceneManager::Get()->EnterPlayMode();

		auto* engine = GalGame::GameEngineCore::GetCurrentEngine();
		if (engine == nullptr)
		{
			out.LastError = "no game engine";
			return false;
		}

		auto* story = engine->GetStoryScriptSystem();
		if (story == nullptr)
		{
			out.LastError = "no story script system";
			return false;
		}

		if (!story->LoadStoryScript(assetPath))
		{
			out.LastError = "LoadStoryScript failed";
			return false;
		}

		auto* executionInstance = story->GetExecutionInstance();
		if (executionInstance == nullptr)
		{
			out.LastError = "no execution instance";
			return false;
		}

		auto* debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
		if (debugInfo == nullptr)
		{
			out.LastError = "no SSSequenceRuntimeDebugInfo";
			return false;
		}

		out.HasValidDebugInfo = true;

		constexpr unsigned kMaxSteps = 500000;
		unsigned stallCount = 0;
		unsigned lastIdx = debugInfo->CurrentIndex;
		for (unsigned step = 0; debugInfo->CurrentIndex < targetEntryIndex; ++step)
		{
			if (step >= kMaxSteps)
			{
				out.LastError = "ExecuteTo step limit exceeded";
				return false;
			}
			executionInstance->Continue();
			executionInstance->Tick(0);
			debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
			if (debugInfo == nullptr)
			{
				out.LastError = "lost debug info during execution";
				return false;
			}
			if (debugInfo->CurrentIndex == lastIdx)
			{
				if (++stallCount > 256)
				{
					out.LastError = "execution stalled (current index did not advance)";
					return false;
				}
			}
			else
			{
				stallCount = 0;
				lastIdx = debugInfo->CurrentIndex;
			}
		}

		out.CurrentIndex = debugInfo->CurrentIndex;
		out.ReachedTarget = true;
		H_LOG_INFO("Executing sequence up to index %u", debugInfo->CurrentIndex);
		return true;
	}
}
