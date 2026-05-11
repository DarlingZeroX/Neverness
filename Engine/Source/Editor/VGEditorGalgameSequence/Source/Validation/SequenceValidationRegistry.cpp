/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/SequenceValidationRegistry.h"

namespace VisionGal::Editor
{
	void SequenceValidationRegistry::Register(std::unique_ptr<ISequenceValidator> validator)
	{
		if (validator != nullptr)
			m_validators.push_back(std::move(validator));
	}

	std::vector<SequenceValidationIssue> SequenceValidationRegistry::RunAll(const SequenceDocument& document) const
	{
		std::vector<SequenceValidationIssue> out;
		for (const auto& v : m_validators)
		{
			if (v == nullptr)
				continue;
			auto part = v->Validate(document);
			out.insert(out.end(), part.begin(), part.end());
		}
		return out;
	}

	std::vector<SequenceValidationIssue> SequenceValidationRegistry::RunForEntries(
		const SequenceDocument& document,
		const std::vector<unsigned>& entryIndices) const
	{
		std::vector<SequenceValidationIssue> out;
		for (const auto& v : m_validators)
		{
			if (v == nullptr)
				continue;
			auto part = v->ValidateEntries(document, entryIndices);
			out.insert(out.end(), part.begin(), part.end());
		}
		return out;
	}
}
