/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/ISequenceValidator.h"

namespace VisionGal::Editor
{
	std::vector<SequenceValidationIssue> ISequenceValidator::ValidateEntries(
		const SequenceDocument& document,
		const std::vector<unsigned>& entryIndices) const
	{
		(void)entryIndices;
		return Validate(document);
	}
}
