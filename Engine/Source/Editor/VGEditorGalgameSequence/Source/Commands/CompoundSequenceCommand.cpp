/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/CompoundSequenceCommand.h"

#include "Document/SequenceDocument.h"

namespace VisionGal::Editor
{
	CompoundSequenceCommand::CompoundSequenceCommand(std::vector<std::unique_ptr<ISequenceEditorCommand>> parts)
		: m_parts(std::move(parts))
	{
	}

	void CompoundSequenceCommand::Execute(SequenceDocument& document)
	{
		for (auto& p : m_parts)
		{
			if (p)
				p->Execute(document);
		}
	}

	void CompoundSequenceCommand::Undo(SequenceDocument& document)
	{
		for (auto it = m_parts.rbegin(); it != m_parts.rend(); ++it)
		{
			if (*it)
				(*it)->Undo(document);
		}
	}

	void CompoundSequenceCommand::Redo(SequenceDocument& document)
	{
		for (auto& p : m_parts)
		{
			if (p)
				p->Redo(document);
		}
	}

	std::string CompoundSequenceCommand::GetDebugName() const
	{
		return "CompoundSequenceCommand";
	}
}
