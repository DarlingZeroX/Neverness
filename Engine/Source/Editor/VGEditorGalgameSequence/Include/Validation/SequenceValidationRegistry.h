/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Validation/ISequenceValidator.h"
#include "Validation/SequenceValidationIssue.h"

#include <memory>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceDocument;

	/// Owns validators and aggregates issues (read-only over document).
	/// 持有校验器并汇总问题（对文档只读）。
	class SequenceValidationRegistry
	{
	public:
		void Register(std::unique_ptr<ISequenceValidator> validator);
		[[nodiscard]] std::vector<SequenceValidationIssue> RunAll(const SequenceDocument& document) const;

	private:
		std::vector<std::unique_ptr<ISequenceValidator>> m_validators;
	};
}
