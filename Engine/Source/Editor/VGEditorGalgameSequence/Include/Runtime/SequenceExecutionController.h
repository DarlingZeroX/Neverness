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
	/// 封装编辑器驱动的播放与单步执行；不缓存引擎执行指针。
	class SequenceExecutionController
	{
	public:
		/// Persists asset, enters play mode if needed, loads script, advances until `targetEntryIndex`.
		/// 持久化资源、必要时进入播放模式、加载脚本并推进到 `targetEntryIndex`。
		bool ExecuteTo(const std::string& assetPath, unsigned int targetEntryIndex, SequenceRuntimeSnapshot& out);
	};
}
