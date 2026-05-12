/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Commands/EditSequencePropertyCommand.h"

#include <string>
#include <vector>

namespace VisionGal::Editor
{
	enum class SequencePropertyKind : uint8_t
	{
		String,
		Bool,
		Int,
		Float,
		Enum,
		AssetRef,
		Color,
		NestedStruct,
		Array,
		CustomDrawer,
	};

	/// Declarative property for palette / auto-inspector (Phase 7).
	struct SequencePropertyDescriptor
	{
		SequencePropertyKind Kind = SequencePropertyKind::String;
		std::string Id;
		std::string Label;
		/// When Kind==String and mapped to undo, use this field id for `EditSequencePropertyCommand`.
		SequenceEditFieldId EditField = SequenceEditFieldId::CommonDialogue_DialogueText;
		bool HasEditField = false;
	};
}
