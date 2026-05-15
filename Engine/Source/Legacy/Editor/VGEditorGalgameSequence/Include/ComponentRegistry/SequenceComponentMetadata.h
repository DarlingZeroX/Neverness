/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequenceComponentSchema.h"

namespace VisionGal::Editor
{
	/// Phase 10：与 `SequenceComponentSchema` 同一类型，保留旧名以降低调用点改动量。
	using SequenceComponentMetadata = SequenceComponentSchema;
}
