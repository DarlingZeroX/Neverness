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
#include <unordered_map>
#include "../HNGEditorCoreConfig.h"
#include "EditorCore.h"
#include "IMNEWrap.h"
#include <HNGRuntimeCore/Include/Core/NodeRegistry.h>
#include "NodeEditorRegistry.h"

namespace Horizon::NodeGraphRuntime
{
	struct RuntimeContext;
}

namespace Horizon::NodeGraphEditor
{
	class CommandManager;
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

		// ----------------------------
		// Editor 节点注册表（独立于 Runtime）
		// - 用于 NodeEditorMeta / PropertyMeta 的默认值初始化
		// - 用于 DrawNodeProperties(ImGui) 自动生成节点属性 UI
		// - 与 Runtime::NodeRegistry 完全独立
		// ----------------------------
		const NodeEditorRegistry* editorRegistry = nullptr;

		bool dirty = true;

		// ----------------------------
		// 辅助索引（O(1) 查找）
		//
		// 说明：
		// - nodeIndexById : NodeId -> nodes 向量下标
		//   用于快速通过 NodeId 查找 EditorNode（替代线性遍历）
		// - pinOwnerById  : PinId  -> NodeId
		//   用于快速判断一个 Pin 属于哪个节点（连线/命令系统/编译器使用）
		//
		// 维护策略：
		// - 在 AddNode / 删除节点 / 反序列化 后调用 RebuildIndices() 或局部更新，
		//   保证索引与 nodes 内容保持一致。
		// - 对于少量节点时性能差异不大，但随着图规模增长，索引能显著降低查找开销。
		// ----------------------------
		std::unordered_map<EditorNodeID, size_t, EditorIdHash> nodeIndexById;
		std::unordered_map<EditorPinID, EditorNodeID, EditorIdHash> pinOwnerById;

		// 命令管理器指针（可选）
		// - 若不为 nullptr，则 UI 操作（如创建连线、移动节点）会通过命令系统执行，
		//   从而支持 Undo/Redo。
		// - 由上层（例如 HNodeGraphEditor）负责设置与生命周期管理。
		CommandManager* commandManager = nullptr;

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
		// 优先通过 nodeIndexById 做 O(1) 查找，发现不一致时会回退线性扫描并修正索引。
		EditorNode* FindNode(ax::NodeEditor::NodeId id);
		// 根据 PinId 在线性表中查找 EditorPin
		// - 先通过 pinOwnerById 找到所属节点，再在该节点的 inputs/outputs 中查找
		// - 若索引缺失或失配，会回退线性扫描并修正 pinOwnerById。
		EditorPin* FindPin(ax::NodeEditor::PinId id);

		// 全量重建辅助索引（在大规模修改或反序列化后调用）
		void RebuildIndices();
	};

	HNG_EDITOR_CORE_API void HandleCreateLink(EditorGraph& graph);
	// 节点删除 / 连线删除入口（内部通过 ImNodeEditor 的 BeginDelete/QueryDeleted...）
	HNG_EDITOR_CORE_API void HandleDelete(EditorGraph& graph);
	// 节点创建菜单（右键弹出）
	HNG_EDITOR_CORE_API void DrawNodeCreateMenu(EditorGraph& graph, const ImVec2& spawnPos);
	HNG_EDITOR_CORE_API void DrawEditorGraph(EditorGraph& graph, const Runtime::RuntimeContext* runtimeCtx = nullptr);
}