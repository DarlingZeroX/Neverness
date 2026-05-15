/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/Builtin/MissingResourcePathValidator.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Document/SequenceDocument.h"

#include "Schema/SequencePropertyFlags.h"
#include "Schema/SequencePropertySchema.h"
#include "Schema/SequencePropertyType.h"
#include "Validation/GenericSchemaValidator.h"

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <unordered_set>

namespace VisionGal::Editor
{
	MissingResourcePathValidator::MissingResourcePathValidator(const SequenceComponentRegistry* componentRegistry)
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
			if (entry == nullptr)
				return;

			if (registry != nullptr)
			{
				if (const SequenceComponentMetadata* meta = registry->Find(entry->GetTypeNameID()))
				{
					for (const SequencePropertySchema& prop : meta->Properties)
					{
						if (prop.Type != SequencePropertyType::ResourcePath)
							continue;
						if (!EnumHasAny(prop.Flags, SequencePropertyFlags::ResourcePathNotEmpty))
							continue;
						if (!prop.Accessor.Getter)
							continue;
						const SequencePropertyValue v = prop.Accessor.Getter(static_cast<void*>(entry));
						for (const SequenceSchemaValidationNote& n : ValidatePropertyValue(prop, v))
						{
							SequenceValidationIssue issue;
							issue.EntryIndex = i;
							issue.Severity = SequenceValidationSeverity::Warning;
							issue.Message = prop.DisplayName.empty() ? n.Message : (prop.DisplayName + std::string{": "} + n.Message);
							if (issue.Message.empty())
								issue.Message = u8"资源路径缺失";
							issue.RuleId = ruleId;
							out.push_back(std::move(issue));
						}
					}
					return;
				}
			}

			if (auto* fig = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(entry))
			{
				if (fig->TextureResourcePath.empty())
				{
					SequenceValidationIssue issue;
					issue.EntryIndex = i;
					issue.Severity = SequenceValidationSeverity::Warning;
					issue.Message = u8"切换立绘缺少纹理资源路径";
					issue.RuleId = ruleId;
					out.push_back(std::move(issue));
				}
			}
			else if (auto* bg = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(entry))
			{
				if (bg->TextureResourcePath.empty())
				{
					SequenceValidationIssue issue;
					issue.EntryIndex = i;
					issue.Severity = SequenceValidationSeverity::Warning;
					issue.Message = u8"切换背景缺少纹理资源路径";
					issue.RuleId = ruleId;
					out.push_back(std::move(issue));
				}
			}
		}
	}

	std::vector<SequenceValidationIssue> MissingResourcePathValidator::Validate(const SequenceDocument& document) const
	{
		std::vector<SequenceValidationIssue> out;
		const unsigned n = document.GetEntryCount();
		for (unsigned i = 0; i < n; ++i)
			PushSchemaOrLegacy(m_componentRegistry, i, const_cast<SequenceDocument&>(document).GetEntryAt(i), out, GetRuleId());
		return out;
	}

	std::vector<SequenceValidationIssue> MissingResourcePathValidator::ValidateEntries(
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
