/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/Builtin/MissingResourcePathValidator.h"

#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		void PushIfEmptyPath(
			std::vector<SequenceValidationIssue>& out, unsigned index, bool emptyPath, const char* label, const char* ruleId)
		{
			if (!emptyPath)
				return;
			SequenceValidationIssue issue;
			issue.EntryIndex = index;
			issue.Severity = SequenceValidationSeverity::Warning;
			issue.Message = label;
			issue.RuleId = ruleId;
			out.push_back(std::move(issue));
		}
	}

	std::vector<SequenceValidationIssue> MissingResourcePathValidator::Validate(const SequenceDocument& document) const
	{
		std::vector<SequenceValidationIssue> out;
		const auto seq = document.GetSequence();
		unsigned i = 0;
		for (const auto& entry : seq->m_Sequence)
		{
			if (auto* fig = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(entry.get()))
				PushIfEmptyPath(out, i, fig->TextureResourcePath.empty(), u8"切换立绘缺少纹理资源路径", GetRuleId());
			else if (auto* bg = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(entry.get()))
				PushIfEmptyPath(out, i, bg->TextureResourcePath.empty(), u8"切换背景缺少纹理资源路径", GetRuleId());
			++i;
		}
		return out;
	}
}
