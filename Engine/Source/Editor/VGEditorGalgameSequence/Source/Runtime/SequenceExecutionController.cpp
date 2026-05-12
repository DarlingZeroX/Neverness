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

#include <algorithm>
#include <limits>

namespace VisionGal::Editor
{
	namespace
	{
		constexpr unsigned kMaxSteps = 500000;

		bool IndexInSortedBreakpoints(const unsigned idx, const std::vector<unsigned>& bp)
		{
			return std::binary_search(bp.begin(), bp.end(), idx);
		}
	}

	bool SequenceExecutionController::EnsurePlayModeAndStory(std::string& err) const
	{
		if (!SceneManager::Get()->IsPlayMode())
			SceneManager::Get()->EnterPlayMode();

		auto* engine = GalGame::GameEngineCore::GetCurrentEngine();
		if (engine == nullptr)
		{
			err = "no game engine";
			return false;
		}

		auto* story = engine->GetStoryScriptSystem();
		if (story == nullptr)
		{
			err = "no story script system";
			return false;
		}
		(void)story;
		return true;
	}

	bool SequenceExecutionController::LoadScriptAndBindDebug(const std::string& assetPath, std::string& err)
	{
		if (!EnsurePlayModeAndStory(err))
			return false;

		auto* engine = GalGame::GameEngineCore::GetCurrentEngine();
		auto* story = engine->GetStoryScriptSystem();

		if (!story->LoadStoryScript(assetPath))
		{
			err = "LoadStoryScript failed";
			return false;
		}

		auto* executionInstance = story->GetExecutionInstance();
		if (executionInstance == nullptr)
		{
			err = "no execution instance";
			return false;
		}

		auto* debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
		if (debugInfo == nullptr)
		{
			err = "no SSSequenceRuntimeDebugInfo";
			return false;
		}
		(void)debugInfo;
		return true;
	}

	bool SequenceExecutionController::BeginDebugSession(const std::string& assetPath, std::string& err)
	{
		err.clear();
		if (assetPath.empty())
		{
			err = "empty asset path";
			return false;
		}

		if (m_debugActive && m_debugAssetPath == assetPath)
			return true;

		EndDebugSession();
		if (!LoadScriptAndBindDebug(assetPath, err))
			return false;

		m_debugActive = true;
		m_debugAssetPath = assetPath;
		return true;
	}

	void SequenceExecutionController::EndDebugSession()
	{
		m_debugActive = false;
		m_debugAssetPath.clear();
	}

	bool SequenceExecutionController::StepOnce(SequenceRuntimeSnapshot& out, std::string& err)
	{
		out = SequenceRuntimeSnapshot{};
		out.CurrentIndex = 0;
		out.HasValidDebugInfo = false;
		err.clear();

		if (!m_debugActive || m_debugAssetPath.empty())
		{
			err = "no active debug session";
			return false;
		}

		if (!EnsurePlayModeAndStory(err))
			return false;

		auto* engine = GalGame::GameEngineCore::GetCurrentEngine();
		auto* story = engine->GetStoryScriptSystem();
		auto* executionInstance = story->GetExecutionInstance();
		if (executionInstance == nullptr)
		{
			err = "no execution instance";
			return false;
		}

		auto* debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
		if (debugInfo == nullptr)
		{
			err = "no SSSequenceRuntimeDebugInfo";
			return false;
		}

		out.HasValidDebugInfo = true;
		const unsigned before = debugInfo->CurrentIndex;

		executionInstance->Continue();
		executionInstance->Tick(0);

		debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
		if (debugInfo == nullptr)
		{
			err = "lost debug info during execution";
			return false;
		}

		out.CurrentIndex = debugInfo->CurrentIndex;
		if (debugInfo->CurrentIndex == before)
			out.StalledNoAdvance = true;
		return true;
	}

	bool SequenceExecutionController::ContinueExecution(
		const unsigned targetEntryIndex,
		const std::vector<unsigned>* sortedBreakpointsOrNull,
		const bool honorPauseFlag,
		bool* inOutPauseRequested,
		SequenceRuntimeSnapshot& out,
		std::string& err)
	{
		out = SequenceRuntimeSnapshot{};
		out.HasValidDebugInfo = false;
		err.clear();

		if (!m_debugActive || m_debugAssetPath.empty())
		{
			err = "no active debug session";
			return false;
		}

		if (!EnsurePlayModeAndStory(err))
			return false;

		auto* engine = GalGame::GameEngineCore::GetCurrentEngine();
		auto* story = engine->GetStoryScriptSystem();
		auto* executionInstance = story->GetExecutionInstance();
		if (executionInstance == nullptr)
		{
			err = "no execution instance";
			return false;
		}

		auto* debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
		if (debugInfo == nullptr)
		{
			err = "no SSSequenceRuntimeDebugInfo";
			return false;
		}

		out.HasValidDebugInfo = true;

		const unsigned kOpenEnded = std::numeric_limits<unsigned>::max();
		const bool openEnded = (targetEntryIndex == kOpenEnded);

		unsigned stallCount = 0;
		unsigned lastIdx = debugInfo->CurrentIndex;

		for (unsigned step = 0;; ++step)
		{
			if (honorPauseFlag && inOutPauseRequested != nullptr && *inOutPauseRequested)
			{
				out.StoppedOnPause = true;
				out.CurrentIndex = debugInfo->CurrentIndex;
				return true;
			}

			if (step >= kMaxSteps)
			{
				err = "ExecuteTo step limit exceeded";
				return false;
			}

			if (!openEnded && debugInfo->CurrentIndex >= targetEntryIndex)
			{
				out.ReachedTarget = true;
				out.CurrentIndex = debugInfo->CurrentIndex;
				H_LOG_INFO("Sequence execution reached index %u", debugInfo->CurrentIndex);
				return true;
			}

			executionInstance->Continue();
			executionInstance->Tick(0);
			debugInfo = executionInstance->ExecutionQuery<GalGame::SSSequenceRuntimeDebugInfo>();
			if (debugInfo == nullptr)
			{
				err = "lost debug info during execution";
				return false;
			}

			if (sortedBreakpointsOrNull != nullptr
				&& IndexInSortedBreakpoints(debugInfo->CurrentIndex, *sortedBreakpointsOrNull))
			{
				out.BreakpointHit = true;
				out.CurrentIndex = debugInfo->CurrentIndex;
				return true;
			}

			if (!openEnded && debugInfo->CurrentIndex >= targetEntryIndex)
			{
				out.ReachedTarget = true;
				out.CurrentIndex = debugInfo->CurrentIndex;
				H_LOG_INFO("Sequence execution reached index %u", debugInfo->CurrentIndex);
				return true;
			}

			if (debugInfo->CurrentIndex == lastIdx)
			{
				if (++stallCount > 256)
				{
					err = "execution stalled (current index did not advance)";
					return false;
				}
			}
			else
			{
				stallCount = 0;
				lastIdx = debugInfo->CurrentIndex;
			}
		}
	}

	bool SequenceExecutionController::ExecuteTo(const std::string& assetPath, const unsigned int targetEntryIndex, SequenceRuntimeSnapshot& out)
	{
		std::string err;
		if (!BeginDebugSession(assetPath, err))
		{
			out = SequenceRuntimeSnapshot{};
			out.LastError = err;
			return false;
		}

		const bool ok = ContinueExecution(targetEntryIndex, nullptr, false, nullptr, out, err);
		EndDebugSession();
		if (!ok)
			out.LastError = err;
		return ok;
	}
}
