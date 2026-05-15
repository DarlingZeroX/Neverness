/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

namespace VisionGal::Editor
{
	class SequenceValidationCacheService;
	class SequenceSearchIndexService;
	class SequenceDebuggerSession;
	class SequenceAsyncTaskService;

	/// Host-owned services exposed to widgets through `SequenceEditorContext`.
	struct SequenceEditorServiceLocator
	{
		SequenceValidationCacheService* validationCache = nullptr;
		SequenceSearchIndexService* searchIndex = nullptr;
		SequenceDebuggerSession* debuggerSession = nullptr;
		SequenceAsyncTaskService* asyncTasks = nullptr;
	};
}
