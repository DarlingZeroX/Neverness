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
	class SequenceExtensionRegistry;

	/// Fills component registry from runtime type IDs with SequenceComponentMetadata (built-in presentation + defaults).
	/// 根据运行时类型 ID 填充 SequenceComponentMetadata（内置展示信息 + 默认值）。
	void BootstrapSequenceComponentRegistry(SequenceComponentRegistry& registry);

	/// Registers inspector adapters (phase-1: legacy drawer bridge) for each registered component type.
	/// 为每种已注册组件类型注册检查器适配器（第一阶段：旧版 Drawer 桥接）。
	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components);

	void BootstrapSequenceExtensions(SequenceExtensionRegistry& registry);
}
