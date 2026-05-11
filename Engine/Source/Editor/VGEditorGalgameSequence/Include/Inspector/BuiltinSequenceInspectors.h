/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "Inspector/ISequenceInspector.h"

#include <memory>

namespace VisionGal::Editor
{
	/// Builds the concrete inspector for a registered sequence component type.
	std::unique_ptr<ISequenceInspector> MakeSequenceInspectorForMetadata(const SequenceComponentMetadata& meta);
}
