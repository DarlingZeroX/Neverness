/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Commands/ISequenceEditorCommand.h"

namespace VisionGal::Editor
{
	SequenceDocumentMutationSummary ISequenceEditorCommand::DescribeExecutedMutation() const
	{
		SequenceDocumentMutationSummary s;
		s.StructuralChange = true;
		return s;
	}

	bool ISequenceEditorCommand::TryMergeWith(ISequenceEditorCommand& incoming, SequenceDocument& document)
	{
		(void)incoming;
		(void)document;
		return false;
	}
}
