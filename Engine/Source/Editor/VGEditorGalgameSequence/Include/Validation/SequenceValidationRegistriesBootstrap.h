/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

namespace VisionGal::Editor
{
	class SequenceComponentRegistry;
	class SequenceValidationRegistry;

	/// @param components 可为空；非空时内置校验器将按 Phase 10 Schema 迭代属性。
	void BootstrapSequenceValidationRegistry(
		SequenceValidationRegistry& registry,
		const SequenceComponentRegistry* components = nullptr);
}
