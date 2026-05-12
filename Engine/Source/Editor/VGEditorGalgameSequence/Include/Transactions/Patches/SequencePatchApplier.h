/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "DirtyRegions/SequenceDirtyRegion.h"
#include "Transactions/Patches/SequenceDocumentPatch.h"

namespace VisionGal::Editor
{
	[[nodiscard]] SequenceDirtyRegion BuildDirtyRegionFromPatchTransaction(const SequencePatchTransactionV2& tx);
}
