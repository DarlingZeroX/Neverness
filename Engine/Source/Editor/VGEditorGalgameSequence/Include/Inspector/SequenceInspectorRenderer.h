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

	/// Phase 10：基于 `SequenceComponentMetadata::Properties` 与 `SequencePropertyAccessor` 的 ImGui 绘制。
	class SequenceInspectorRenderer
	{
	public:
		/// @return 是否至少绘制了一个可编辑或可见字段。
		static bool DrawFromSchema(
			const SequenceComponentMetadata& schema,
			unsigned entryIndex,
			void* component,
			SequenceEditorContext* context);

		/// 兼容旧名。
		static bool DrawFromDescriptors(
			const SequenceComponentMetadata& meta,
			unsigned entryIndex,
			VisionGal::IVGSSequenceComponent* component,
			SequenceEditorContext* context);
	};
}
