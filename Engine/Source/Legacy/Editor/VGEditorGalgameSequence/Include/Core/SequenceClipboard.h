/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <NNKernel/Interface/HConfig.h>
#include <cstdint>
#include <vector>

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// Editor clipboard: deep copies of selected entries for cut/copy/paste with undo.
	/// 编辑器剪贴板：选中条目的深拷贝，用于剪切/复制/粘贴并支持撤销。
	class SequenceClipboard
	{
	public:
		void Clear();

		/// Replaces clipboard with clones of the given sorted unique indices (ascending).
		/// 用给定已排序且不重复索引（升序）对应条目的克隆替换剪贴板内容。
		void SetFromSelection(const SequenceEditorContext& context, const std::vector<uint32_t>& sortedUniqueIndices);

		/// Copy selection to clipboard (no document change).
		/// 将选中内容复制到剪贴板（不修改文档）。
		void CopySelection(SequenceEditorContext& context);

		/// Copy then remove selected entries (single undo via remove command — use Compound with paste for full cut semantics).
		/// 复制后删除选中条目（通过删除命令单次撤销；完整剪切语义需与粘贴等组合为复合命令）。
		void CutSelection(SequenceEditorContext& context);

		/// Inserts cloned clipboard entries at the computed insert index (undoable).
		/// 在计算得到的插入索引处插入剪贴板条目的克隆（可撤销）。
		void TryPaste(SequenceEditorContext& context);

		bool HasContent() const { return !m_entries.empty(); }

	private:
		static unsigned ComputePasteInsertIndex(const SequenceEditorContext& context);

		std::vector<Ref<VisionGal::IVGSSequenceComponent>> m_entries;
	};
}
