/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/Builtin/EmptyDialogueValidator.h"

#include "Document/SequenceDocument.h"

#include <unordered_set>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	std::vector<SequenceValidationIssue> EmptyDialogueValidator::Validate(const SequenceDocument& document) const
	{
		std::vector<SequenceValidationIssue> out;
		const unsigned n = document.GetEntryCount();
		for (unsigned i = 0; i < n; ++i)
		{
			auto* dialogue = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(const_cast<SequenceDocument&>(document).GetEntryAt(i));
			if (dialogue != nullptr && dialogue->DialogueText.empty())
			{
				SequenceValidationIssue issue;
				issue.EntryIndex = i;
				issue.Severity = SequenceValidationSeverity::Warning;
				issue.Message = u8"普通对话文本为空";
				issue.RuleId = GetRuleId();
				out.push_back(std::move(issue));
			}
		}
		return out;
	}

	std::vector<SequenceValidationIssue> EmptyDialogueValidator::ValidateEntries(
		const SequenceDocument& document,
		const std::vector<unsigned>& entryIndices) const
	{
		std::unordered_set<unsigned> want(entryIndices.begin(), entryIndices.end());
		std::vector<SequenceValidationIssue> out;
		const unsigned n = document.GetEntryCount();
		for (unsigned i : want)
		{
			if (i >= n)
				continue;
			auto* dialogue = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(const_cast<SequenceDocument&>(document).GetEntryAt(i));
			if (dialogue != nullptr && dialogue->DialogueText.empty())
			{
				SequenceValidationIssue issue;
				issue.EntryIndex = i;
				issue.Severity = SequenceValidationSeverity::Warning;
				issue.Message = u8"普通对话文本为空";
				issue.RuleId = GetRuleId();
				out.push_back(std::move(issue));
			}
		}
		return out;
	}
}
