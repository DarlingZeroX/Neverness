/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Runtime/SequenceRuntimeSnapshot.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	/// Encapsulates editor-driven playback / stepping. Optional debug session caches loaded script.
	class SequenceExecutionController
	{
	public:
		bool ExecuteTo(const std::string& assetPath, unsigned int targetEntryIndex, SequenceRuntimeSnapshot& out);

		bool BeginDebugSession(const std::string& assetPath, std::string& err);
		void EndDebugSession();

		[[nodiscard]] bool IsDebugSessionActive() const { return m_debugActive; }

		bool StepOnce(SequenceRuntimeSnapshot& out, std::string& err);

		/// Advances until `targetEntryIndex`, a breakpoint index, stall, optional user pause between steps, or step cap.
		bool ContinueExecution(
			unsigned targetEntryIndex,
			const std::vector<unsigned>* sortedBreakpointsOrNull,
			bool honorPauseFlag,
			bool* inOutPauseRequested,
			SequenceRuntimeSnapshot& out,
			std::string& err);

	private:
		bool EnsurePlayModeAndStory(std::string& err) const;
		bool LoadScriptAndBindDebug(const std::string& assetPath, std::string& err);

		bool m_debugActive = false;
		std::string m_debugAssetPath;
	};
}
