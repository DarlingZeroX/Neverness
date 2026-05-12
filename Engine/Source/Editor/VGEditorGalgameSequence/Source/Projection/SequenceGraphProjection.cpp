/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceGraphProjection.h"

#include "AuthoringGraph/SequenceAuthoringGraph.h"
#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "DirtyRegions/SequenceDirtyRegionFlags.h"
#include "Document/SequenceDocument.h"
#include "Projection/SequenceProjectionContext.h"

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		std::string BuildSubtitle(const VisionGal::IVGSSequenceComponent* entry)
		{
			if (entry == nullptr)
				return {};
			if (auto* d = dynamic_cast<const VisionGal::VGSSC_CommonDialogue*>(entry))
			{
				if (!d->DialogueCharacterName.empty())
					return d->DialogueCharacterName + " — " + d->DialogueText;
				return d->DialogueText;
			}
			if (auto* f = dynamic_cast<const VisionGal::VGSSC_ChangeFigure*>(entry))
				return f->TextureResourcePath;
			if (auto* b = dynamic_cast<const VisionGal::VGSSC_ChangeBackground*>(entry))
				return b->TextureResourcePath;
			return {};
		}
	}

	void SequenceGraphProjection::Rebuild(const SequenceProjectionContext& ctx)
	{
		if (ctx.document == nullptr || ctx.registry == nullptr)
			return;
		SequenceDocument& document = *ctx.document;
		const SequenceComponentRegistry& registry = *ctx.registry;
		m_nodes.clear();
		m_edges.clear();
		const unsigned n = document.GetEntryCount();
		m_nodes.reserve(n);
		for (unsigned i = 0; i < n; ++i)
		{
			const VisionGal::IVGSSequenceComponent* entry = document.GetEntryAt(i);
			SequenceGraphNodeVM node;
			node.EntryIndex = i;
			if (entry != nullptr)
				node.TypeNameID = const_cast<VisionGal::IVGSSequenceComponent*>(entry)->GetTypeNameID();
			if (const SequenceComponentMetadata* meta = registry.Find(node.TypeNameID))
				node.Title = meta->PrimaryLabel();
			else
				node.Title = node.TypeNameID;
			node.Subtitle = BuildSubtitle(entry);
			node.LayoutX = static_cast<float>(i) * 200.f;
			node.LayoutY = static_cast<float>((i % 8) * 72.f);
			if (m_authoringGraph != nullptr)
			{
				m_authoringGraph->EnsureNodeForEntry(i, node.LayoutX, node.LayoutY);
				SequenceAuthoringNode an;
				if (m_authoringGraph->TryGetNodeForEntry(i, an))
				{
					node.LayoutX = an.posX;
					node.LayoutY = an.posY;
				}
			}
			m_nodes.push_back(std::move(node));
		}
		for (unsigned i = 0; i + 1 < n; ++i)
		{
			SequenceGraphEdgeVM e;
			e.FromEntryIndex = i;
			e.ToEntryIndex = i + 1;
			e.Kind = SequenceGraphEdgeKind::LinearNext;
			m_edges.push_back(e);
		}
	}

	void SequenceGraphProjection::ApplyDirtyRegion(
		const SequenceDirtyRegion& dirty,
		const SequenceProjectionContext& ctx)
	{
		const bool structural =
			(dirty.Flags & SequenceDirtyRegionFlags::Structure) != SequenceDirtyRegionFlags::None;
		const bool property =
			(dirty.Flags & SequenceDirtyRegionFlags::Property) != SequenceDirtyRegionFlags::None;
		if (structural || (property && !dirty.Entries.empty()))
			Rebuild(ctx);
	}
}
