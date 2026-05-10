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
	class SequenceInspectorRegistry;

	/// Fills component registry with built-in sequence types (metadata only; UI rows use EntryUIDataManager).
	void BootstrapSequenceComponentRegistry(SequenceComponentRegistry& registry);

	/// Registers inspector adapters (phase-1: legacy drawer bridge) for each registered component type.
	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components);
}
