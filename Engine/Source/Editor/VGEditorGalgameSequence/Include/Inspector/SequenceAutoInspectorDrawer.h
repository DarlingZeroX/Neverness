/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ComponentRegistry/SequenceComponentMetadata.h"

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Fallback inspector driven by `SequenceComponentMetadata::PropertyDescriptors`.
	bool TryDrawAutoInspectorFromDescriptors(
		const SequenceComponentMetadata& meta,
		unsigned index,
		VisionGal::IVGSSequenceComponent* component,
		SequenceEditorContext* context);
}
