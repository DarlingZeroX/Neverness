/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Document/SequenceDocument.h"

#include "VGGalgameScriptSequence/Include/Asset/Asset.h"
#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	SequenceDocument::SequenceDocument()
	{
		m_sequence = MakeRef<VisionGal::VGSSequenceDataContainer>();
	}

	SequenceDocument::SequenceDocument(
		Ref<VisionGal::VGSSequenceDataContainer> sequence,
		SequenceDocumentValidationSnapshotTag)
		: m_sequence(std::move(sequence))
	{
	}

	unsigned SequenceDocument::GetEntryCount() const
	{
		return static_cast<unsigned>(m_sequence->m_Sequence.size());
	}

	VisionGal::IVGSSequenceComponent* SequenceDocument::GetEntryAt(unsigned index)
	{
		if (index >= m_sequence->m_Sequence.size())
			return nullptr;
		return m_sequence->m_Sequence[index].get();
	}

	const VisionGal::IVGSSequenceComponent* SequenceDocument::GetEntryAt(unsigned index) const
	{
		if (index >= m_sequence->m_Sequence.size())
			return nullptr;
		return m_sequence->m_Sequence[index].get();
	}

	Ref<VisionGal::VGSSequenceDataContainer> SequenceDocument::CloneSequenceDeepForValidation() const
	{
		auto out = MakeRef<VisionGal::VGSSequenceDataContainer>();
		out->m_Sequence.reserve(m_sequence->m_Sequence.size());
		for (const auto& e : m_sequence->m_Sequence)
		{
			if (e)
				out->m_Sequence.push_back(e->Clone());
			else
				out->m_Sequence.push_back(nullptr);
		}
		return out;
	}

	void SequenceDocument::BumpEditGeneration()
	{
		++m_generationId;
	}

	void SequenceDocument::BumpStructureRevision()
	{
		++m_structureRevision;
		++m_generationId;
	}

	void SequenceDocument::ResetSequenceData()
	{
		m_sequence = MakeRef<VisionGal::VGSSequenceDataContainer>();
		MarkDirty();
		BumpStructureRevision();
	}

	void SequenceDocument::FillDefaultDemoEntries()
	{
		m_sequence->AppendEntry(VisionGal::CreateSequenceEntryByTypeNameID(VGSSC_CommonDialogue::StaticGetTypeNameID()));
		m_sequence->AppendEntry(VisionGal::CreateSequenceEntryByTypeNameID(VGSSC_ChangeFigure::StaticGetTypeNameID()));
		m_sequence->AppendEntry(VisionGal::CreateSequenceEntryByTypeNameID(VGSSC_ChangeBackground::StaticGetTypeNameID()));
		m_sequence->AppendEntry(VisionGal::CreateSequenceEntryByTypeNameID(VGSSC_CommonDialogue::StaticGetTypeNameID()));
		MarkDirty();
		BumpStructureRevision();
	}

	void SequenceDocument::MarkDirty()
	{
		m_dirty = true;
	}

	void SequenceDocument::ClearDirty()
	{
		m_dirty = false;
	}

	bool SequenceDocument::LoadFromAssetPath(const std::string& path)
	{
		GalGame::SequenceScriptAssetLoader loader;
		Ref<VGAsset> asset = nullptr;
		if (!loader.Read(path, asset))
			return false;
		if (asset == nullptr)
			return false;

		Ref<GalGame::SequenceScriptAsset> scriptAsset = std::dynamic_pointer_cast<GalGame::SequenceScriptAsset>(asset);
		if (scriptAsset == nullptr)
			return false;

		m_sequence = scriptAsset->ExecutionData->SequenceData;
		m_assetPath = path;
		ClearDirty();
		BumpStructureRevision();
		return true;
	}

	bool SequenceDocument::SaveToAssetPath()
	{
		if (m_assetPath.empty())
			return false;

		GalGame::SequenceScriptAssetWriter writer;
		auto asset = MakeRef<GalGame::SequenceScriptAsset>();
		asset->ExecutionData->SequenceData = m_sequence;
		writer.Write(m_assetPath, asset.get());
		ClearDirty();
		return true;
	}

	bool SequenceDocument::SaveAsToAssetPath(const std::string& path)
	{
		if (path.empty())
			return false;
		m_assetPath = path;
		return SaveToAssetPath();
	}

	void SequenceDocument::ResetToUntitledEmpty()
	{
		m_assetPath.clear();
		m_sequence = MakeRef<VisionGal::VGSSequenceDataContainer>();
		MarkDirty();
		BumpStructureRevision();
	}

	void SequenceDocument::AddEntryByTypeNameID(const std::string& typeNameID)
	{
		m_sequence->AppendEntry(VisionGal::CreateSequenceEntryByTypeNameID(typeNameID));
		MarkDirty();
	}

	void SequenceDocument::RemoveEntries(const std::vector<unsigned>& indices)
	{
		m_sequence->RemoveEntries(indices);
		MarkDirty();
	}

	bool SequenceDocument::ReorderInsertBefore(unsigned sourceIndex, unsigned targetIndex)
	{
		const bool ok = m_sequence->InsertBefore(sourceIndex, targetIndex);
		if (ok)
			MarkDirty();
		return ok;
	}

	bool SequenceDocument::ReorderInsertBehind(unsigned sourceIndex, unsigned targetIndex)
	{
		const bool ok = m_sequence->InsertBehind(sourceIndex, targetIndex);
		if (ok)
			MarkDirty();
		return ok;
	}
}
