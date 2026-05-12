/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertySchema.h"

namespace VisionGal::Editor
{
	/// Phase 10：请优先使用 `SequencePropertySchema`；本别名仅为兼容旧源码与文档引用。
	using SequencePropertyDescriptor = SequencePropertySchema;
}
