/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <memory>
#include <string>

namespace VisionGal::Editor
{
	class SequenceDocument;

	class ISequenceEditorCommand
	{
	public:
		virtual ~ISequenceEditorCommand() = default;

		virtual void Execute(SequenceDocument& document) = 0;
		virtual void Undo(SequenceDocument& document) = 0;
		virtual void Redo(SequenceDocument& document) = 0;

		virtual std::string GetDebugName() const = 0;
	};
}
