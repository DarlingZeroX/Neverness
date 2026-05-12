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
	class SequenceDocument;
	class SequenceRuntimeOverlayState;
	class SequenceSearchIndexService;
	class SequenceSelectionModel;
	class SequenceValidationCacheService;

	/// Phase 9：投影层只读依赖收口。所有 Document→ViewModel 的投影 Rebuild / ApplyDirtyRegion
	/// 仅允许从此结构取值，禁止反向持有 `VGScriptSequenceEditor` 或 `SequenceEditorContext`。
	/// 在规格基础上增加 `registry`：各投影仍依赖组件元数据；Authoring 图仍由
	/// `SequenceGraphProjection::SetAuthoringGraph` 单独注入，避免上下文膨胀。
	struct SequenceProjectionContext
	{
		SequenceDocument* document = nullptr;
		const SequenceValidationCacheService* validation = nullptr;
		const SequenceRuntimeOverlayState* runtime = nullptr;
		const SequenceSearchIndexService* search = nullptr;
		const SequenceSelectionModel* selection = nullptr;
		const SequenceComponentRegistry* registry = nullptr;
	};

	/// 单测或仅含文档+注册表的代码路径使用的最小上下文。
	inline SequenceProjectionContext MakeSequenceProjectionContext(
		SequenceDocument& document,
		const SequenceComponentRegistry& registry)
	{
		SequenceProjectionContext c;
		c.document = &document;
		c.registry = &registry;
		return c;
	}
}
