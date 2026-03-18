/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include <string>
#include "../HNGEditorCoreConfig.h"
#include "EditorCore.h"
#include "IMNEWrap.h"
#include <HNGRuntimeCore/Include/Core/NodeRegistry.h>

namespace Horizon::NodeGraphRuntime
{
	struct RuntimeContext;
}

namespace Horizon::NodeGraphEditor
{
	namespace Runtime = Horizon::NodeGraphRuntime;

	struct HNG_EDITOR_CORE_API EditorGraph
	{
		std::vector<EditorNode> nodes;
		std::vector<EditorLink> links;

		Ref<IMNEEditorContext> context = nullptr;

		// ----------------------------
		// 运行时节点注册表（元数据来源）
		// 说明：
		// - Editor 侧不再“手写 pins”，所有节点结构都必须来自 NodeRegistry::NodeMeta
		// - 这里保存 registry 指针，便于 AddNode(NodeType) 一步创建节点
		// - registry 的生命周期由上层（例如 HNodeGraphEditor）保证
		// ----------------------------
		const Runtime::NodeRegistry* registry = nullptr;

		bool dirty = true;

		// ----------------------------
		// AddNode：以 NodeType 为输入，自动构建 EditorNode
		//
		// 核心逻辑：
		// - 从 registry 取 NodeMeta
		// - 调用 CreateNodeFromMeta(meta) 自动生成 inputs/outputs pins
		// - 将节点加入 graph.nodes，并将 dirty 置为 true 触发热编译
		//
		// 注意：
		// - 若 registry/meta 不存在，会创建一个最小的占位节点（无 pins），避免崩溃
		// ----------------------------
		EditorNode& AddNode(Runtime::NodeType type);

		// 根据 NodeId 在线性表中查找 EditorNode
		EditorNode* FindNode(ax::NodeEditor::NodeId id);
		// 根据 PinId 在线性表中查找 EditorPin
		EditorPin* FindPin(ax::NodeEditor::PinId id);
	};

	HNG_EDITOR_CORE_API void HandleCreateLink(EditorGraph& graph);
	HNG_EDITOR_CORE_API void DrawEditorGraph(EditorGraph& graph, const Runtime::RuntimeContext* runtimeCtx = nullptr);
}