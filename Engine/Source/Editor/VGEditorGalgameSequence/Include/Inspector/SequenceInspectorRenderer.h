/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	struct SequenceComponentMetadata;
	struct SequenceEditorContext;

	/// Phase 9：100% 基于 `SequenceComponentMetadata::PropertyDescriptors` 的 ImGui 属性绘制。
	/// 无描述符时由调用方显示错误提示；不再提供「无描述符回退」路径。
	class SequenceInspectorRenderer
	{
	public:
		/// @return 是否至少绘制了一个可编辑或可见字段。
		static bool DrawFromDescriptors(
			const SequenceComponentMetadata& meta,
			unsigned entryIndex,
			VisionGal::IVGSSequenceComponent* component,
			SequenceEditorContext* context);
	};
}
