/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Async/SequenceTaskToken.h"

namespace VisionGal::Editor
{
	std::atomic<std::uint64_t> SequenceTaskToken::s_nextId{1};
}
