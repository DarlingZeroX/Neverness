/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/PropertyEditing/SequencePropertyBindingRegistry.h"

#include "Inspector/PropertyEditing/SequencePropertyPath.h"

#include <string>

namespace VisionGal::Editor
{
	const std::vector<SequencePropertyBinding>& BuiltinBindingsForCommonDialogue()
	{
		static const std::vector<SequencePropertyBinding> kBindings = {
			{std::string(SequencePropertyPath::kCommonDialogueCharacterName), u8"角色名", SequencePropertyBindingKind::String},
			{std::string(SequencePropertyPath::kCommonDialogueText), u8"对话文本", SequencePropertyBindingKind::String},
		};
		return kBindings;
	}
}
