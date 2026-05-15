/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/Builtin/EmptyDialogueValidator.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Document/SequenceDocument.h"

#include "Schema/SequencePropertySchema.h"
#include "Validation/GenericSchemaValidator.h"

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <unordered_set>

namespace VisionGal::Editor
{
	EmptyDialogueValidator::EmptyDialogueValidator(const SequenceComponentRegistry* componentRegistry)
		: m_componentRegistry(componentRegistry)
	{
	}

	namespace
	{
		void PushSchemaOrLegacy(
			const SequenceComponentRegistry* registry,
			const unsigned i,
			VisionGal::IVGSSequenceComponent* entry,
			std::vector<SequenceValidationIssue>& out,
			const char* ruleId)
		{
			auto* dialogue = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(entry);
			if (dialogue == nullptr)
				return;

			if (registry != nullptr)
			{
				if (const SequenceComponentMetadata* meta = registry->Find(VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID()))
				{
					for (const SequencePropertySchema& prop : meta->Properties)
					{
						if (prop.Name != "dialogue" || !prop.Accessor.Getter)
							continue;
						const SequencePropertyValue v = prop.Accessor.Getter(static_cast<void*>(dialogue));
						for (const SequenceSchemaValidationNote& n : ValidatePropertyValue(prop, v))
						{
							SequenceValidationIssue issue;
							issue.EntryIndex = i;
							issue.Severity = SequenceValidationSeverity::Warning;
							issue.Message = n.Message.empty() ? std::string{u8"普通对话文本为空"} : n.Message;
							issue.RuleId = ruleId;
							out.push_back(std::move(issue));
						}
					}
					return;
				}
			}

			if (dialogue->DialogueText.empty())
			{
				SequenceValidationIssue issue;
				issue.EntryIndex = i;
				issue.Severity = SequenceValidationSeverity::Warning;
				issue.Message = u8"普通对话文本为空";
				issue.RuleId = ruleId;
				out.push_back(std::move(issue));
			}
		}
	}

	std::vector<SequenceValidationIssue> EmptyDialogueValidator::Validate(const SequenceDocument& document) const
	{
		std::vector<SequenceValidationIssue> out;
		const unsigned n = document.GetEntryCount();
		for (unsigned i = 0; i < n; ++i)
			PushSchemaOrLegacy(m_componentRegistry, i, const_cast<SequenceDocument&>(document).GetEntryAt(i), out, GetRuleId());
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
			PushSchemaOrLegacy(m_componentRegistry, i, const_cast<SequenceDocument&>(document).GetEntryAt(i), out, GetRuleId());
		}
		return out;
	}
}
