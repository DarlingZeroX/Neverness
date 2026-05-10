/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Core/SequenceEditorContext.h"

#include "Commands/ISequenceEditorCommand.h"
#include "Core/SequenceUndoStack.h"
#include "Document/SequenceDocument.h"

#include "HCore/Interface/HLog.h"

namespace VisionGal::Editor
{
	void SequenceEditorContext::ExecuteCommand(std::unique_ptr<ISequenceEditorCommand> command)
	{
		if (command == nullptr)
			return;
		if (undo == nullptr || document == nullptr)
		{
			H_LOG_WARN("SequenceEditorContext::ExecuteCommand ignored: missing undo stack or document");
			return;
		}
		undo->ExecuteCommand(std::move(command), *document);
	}
}
