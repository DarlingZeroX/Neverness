/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Projection/SequenceGraphProjection.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Document/SequenceDocument.h"
#include "ViewModels/SequenceDocumentViewModel.h"

namespace VisionGal::Editor
{
	void SequenceGraphProjection::Apply(
		const bool seedPresentation,
		const SequenceDirtyRegion& dirty,
		SequenceDocument& document,
		SequenceDocumentViewModel& viewModel,
		SequenceComponentRegistry& registry)
	{
		(void)seedPresentation;
		(void)dirty;
		(void)document;
		(void)viewModel;
		(void)registry;
	}
}
