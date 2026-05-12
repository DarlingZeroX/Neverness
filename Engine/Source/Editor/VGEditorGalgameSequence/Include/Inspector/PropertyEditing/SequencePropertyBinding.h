/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	enum class SequencePropertyBindingKind : uint8_t
	{
		String,
		Bool,
	};

	/// Descriptor for generic inspector / binding layer (Phase 6).
	struct SequencePropertyBinding
	{
		std::string Path;
		std::string UiLabel;
		SequencePropertyBindingKind Kind = SequencePropertyBindingKind::String;
	};
}
