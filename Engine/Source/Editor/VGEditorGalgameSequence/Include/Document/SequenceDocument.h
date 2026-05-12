/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <HCore/Interface/HConfig.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "VGGalgameScriptSequence/Include/Sequence/DataContainer.h"

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	struct SequenceDocumentValidationSnapshotTag
	{
	};

	/// Owns asset path, dirty flag, and the edited sequence (thin wrapper over `VGSSequenceDataContainer`).
	/// 持有资源路径、脏标记与正在编辑的序列（对 `VGSSequenceDataContainer` 的薄封装）。
	class SequenceDocument
	{
	public:
		SequenceDocument();

		/// Ephemeral document for validation on a cloned sequence (worker thread). Not for undo stack.
		explicit SequenceDocument(
			Ref<VisionGal::VGSSequenceDataContainer> sequence,
			SequenceDocumentValidationSnapshotTag);

		[[nodiscard]] unsigned GetEntryCount() const;
		[[nodiscard]] VisionGal::IVGSSequenceComponent* GetEntryAt(unsigned index);
		[[nodiscard]] const VisionGal::IVGSSequenceComponent* GetEntryAt(unsigned index) const;

		/// Shallow copy of current entry list (shared component ownership), e.g. move-order undo snapshot.
		[[nodiscard]] std::vector<Ref<VisionGal::IVGSSequenceComponent>> CopyEntryRefVector() const
		{
			return m_sequence->m_Sequence;
		}

		/// Deep clone of all entries for async validation (worker must not touch the live document).
		[[nodiscard]] Ref<VisionGal::VGSSequenceDataContainer> CloneSequenceDeepForValidation() const;

		const std::string& GetAssetPath() const { return m_assetPath; }

		[[nodiscard]] uint64_t GetGenerationId() const { return m_generationId; }
		[[nodiscard]] uint64_t GetStructureRevision() const { return m_structureRevision; }
		void BumpEditGeneration();
		void BumpStructureRevision();

		bool IsDirty() const { return m_dirty; }
		void MarkDirty();
		void ClearDirty();

		/// Creates a new empty sequence container.
		/// 创建新的空序列容器。
		void ResetSequenceData();

		/// Fills default demo entries (matches previous editor ctor behaviour).
		/// 填充默认演示条目（与旧版编辑器构造函数行为一致）。
		void FillDefaultDemoEntries();

		bool LoadFromAssetPath(const std::string& path);
		bool SaveToAssetPath();

		/// Sets `m_assetPath` then saves. Returns false if `path` is empty or write fails.
		/// 设置 `m_assetPath` 后保存；若 `path` 为空或写入失败则返回 false。
		bool SaveAsToAssetPath(const std::string& path);

		/// Clears asset path and replaces sequence with an empty container (untitled new document).
		/// 清空资源路径并将序列替换为空容器（未命名新文档）。
		void ResetToUntitledEmpty();

		void AddEntryByTypeNameID(const std::string& typeNameID);
		void RemoveEntries(const std::vector<unsigned>& indices);
		bool ReorderInsertBefore(unsigned sourceIndex, unsigned targetIndex);
		bool ReorderInsertBehind(unsigned sourceIndex, unsigned targetIndex);

		/// Restores full ordered entry list (undo helpers / move snapshot).
		/// 恢复完整有序条目列表（撤销辅助 / 移动快照）。
		void SetSequenceEntries(const std::vector<Ref<VisionGal::IVGSSequenceComponent>>& entries)
		{
			m_sequence->m_Sequence = entries;
			for (unsigned i = 0; i < m_sequence->m_Sequence.size(); ++i)
				m_sequence->m_Sequence[i]->SequenceIndex = i;
			MarkDirty();
		}

		/// Inserts a single entry at `index` (clamped to end). Reindexes `SequenceIndex`.
		/// 在 `index` 处插入单条条目（越界则夹到末尾），并重新编号 `SequenceIndex`。
		void InsertEntryAt(unsigned index, Ref<VisionGal::IVGSSequenceComponent> entry)
		{
			auto& vec = m_sequence->m_Sequence;
			if (index > vec.size())
				index = static_cast<unsigned>(vec.size());
			vec.insert(vec.begin() + static_cast<std::ptrdiff_t>(index), std::move(entry));
			for (unsigned i = 0; i < vec.size(); ++i)
				vec[i]->SequenceIndex = i;
			MarkDirty();
		}

		/// Internal: asset I/O and command implementations in this module.
		[[nodiscard]] Ref<VisionGal::VGSSequenceDataContainer> GetSequenceDataMutable() const { return m_sequence; }

	private:
		std::string m_assetPath;
		Ref<VisionGal::VGSSequenceDataContainer> m_sequence;
		bool m_dirty = false;
		uint64_t m_generationId = 1;
		uint64_t m_structureRevision = 1;
	};
}
