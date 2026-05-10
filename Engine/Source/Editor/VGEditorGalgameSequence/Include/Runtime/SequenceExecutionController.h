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

namespace VisionGal::Editor
{
	/// Encapsulates editor-driven playback / stepping. Does not cache engine execution pointers.
	class SequenceExecutionController
	{
	public:
		/// Persists asset, enters play mode if needed, loads script, advances until `targetEntryIndex`.
		bool ExecuteTo(const std::string& assetPath, unsigned int targetEntryIndex, SequenceRuntimeSnapshot& out);
	};
}
