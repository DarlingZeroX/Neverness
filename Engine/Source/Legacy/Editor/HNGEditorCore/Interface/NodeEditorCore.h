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
#include <functional>

#include "../HNGEditorCoreConfig.h"

#include <NNRuntimeImGui/Include/ImNodeEditor/imgui_node_editor.h>
#include <NNNodeGraphCore/Interface/RuntimeCore.h>
//namespace IMNE = ax::NodeEditor;

namespace Horizon::NodeGraphEditor
{
	struct EditorIdHash
	{
		template<typename T>
		size_t operator()(const T& id) const noexcept
		{
			// ax::NodeEditor::NodeId / PinId / LinkId 都提供 Get() -> int
			return std::hash<int>()(id.Get());
		}
	};

	// ---------------------------------------------------------------------
	// 允许在项目中直接写：
	//   std::unordered_set<ax::NodeEditor::NodeId>
	//   std::unordered_set<ax::NodeEditor::LinkId>
	//   std::unordered_set<ax::NodeEditor::PinId>
	//
	// 原因：ax::NodeEditor 的 NodeId/LinkId/PinId 是自定义 struct，
	// 默认 std::unordered_set 无法自动推导 hash，需要提供 std::hash 特化。
	// ---------------------------------------------------------------------

} // namespace Horizon::NodeGraphEditor

// ---------------------------------------------------------------------
// std::hash 特化（让 EditorGraph 能使用 unordered_set<NodeId> 的“纯类型”写法）
// ---------------------------------------------------------------------
namespace std
{
	template<>
	struct hash<ax::NodeEditor::NodeId>
	{
		size_t operator()(const ax::NodeEditor::NodeId& id) const noexcept
		{
			return std::hash<int>()(id.Get());
		}
	};

	template<>
	struct hash<ax::NodeEditor::LinkId>
	{
		size_t operator()(const ax::NodeEditor::LinkId& id) const noexcept
		{
			return std::hash<int>()(id.Get());
		}
	};

	template<>
	struct hash<ax::NodeEditor::PinId>
	{
		size_t operator()(const ax::NodeEditor::PinId& id) const noexcept
		{
			return std::hash<int>()(id.Get());
		}
	};
}

namespace Horizon::NodeGraphEditor
{

	using EditorNodeID = ax::NodeEditor::NodeId;
	using EditorLinkID = ax::NodeEditor::LinkId;
	using EditorPinID = ax::NodeEditor::PinId;

	struct EditorPin
	{
		ax::NodeEditor::PinId id;

		std::string name;
		NodeGraphRuntime::SlotType type;

		bool isInput;

		// UI
		ImColor color;

		// 编译用
		uint32_t runtimeIndex = 0;
	};

	struct EditorNode
	{
		EditorNodeID id;

		NodeGraphRuntime::NodeTypeId typeId;
		std::string name;

		std::vector<EditorPin> inputs;
		std::vector<EditorPin> outputs;

		ImVec2 position;

		// 自定义数据（用于 Dialogue 等）
		std::unordered_map<std::string, NodeGraphRuntime::Value> properties;

		// 属性访问封装：
		// - 不存在则创建一个默认 Value（type=ValueType::None）
		// - UI 不应直接操作 properties，优先走该函数
		NodeGraphRuntime::Value& GetProperty(const std::string& name)
		{
			auto it = properties.find(name);
			if (it == properties.end())
			{
				it = properties.emplace(name, NodeGraphRuntime::Value{}).first;
			}
			return it->second;
		}
	};

	struct EditorLink
	{
		EditorLinkID id;
		
		EditorPinID startPinId;
		EditorPinID endPinId;
	};
}

