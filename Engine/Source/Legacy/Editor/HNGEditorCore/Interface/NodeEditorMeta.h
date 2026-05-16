/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

#include <HNGRuntimeCore/Interface/Types.h>
#include <HNGRuntimeCore/Interface/Value.h>

namespace Horizon::NodeGraphEditor
{
	// 属性控件类型（仅用于 EditorCore 的 UI 渲染）
	enum class PropertyWidgetType
	{
		InputText,
		MultilineText,
		Checkbox,
		Float,
		Int,
		Combo
	};

	// 节点属性描述（用于：默认值初始化 + 自动生成 ImGui 控件）
	struct PropertyMeta
	{
		std::string name;			// 内部 key（写入 EditorNode.properties[name]）
		std::string displayName;	// UI 展示 label

		NodeGraphRuntime::ValueType type;	// Value 类型
		PropertyWidgetType widget;			// 控件类型
		NodeGraphRuntime::Value defaultValue;

		// Combo 用选项（widget==Combo 时使用）
		std::vector<std::string> options;
	};

	// Editor 专用节点元数据（与 Runtime NodeMeta 完全独立）
	struct NodeEditorMeta
	{
		NodeGraphRuntime::NodeTypeId typeId;

		// UI 展示
		std::string displayName;
		std::string category;

		// 属性描述
		std::vector<PropertyMeta> properties;

		// 自定义节点 UI（可选）：
		// - 用于复杂节点（如 DialogueList / Timeline / SkillGraph）的专用编辑器面板
		// - Core 不内置任何业务 UI；业务模块通过 NodeEditorRegistry 注册此回调
		// - 回调内部可自由使用 ImGui，并在需要时设置 graph.dirty = true
		//
		// 注意：DrawSingleNodeImpl 会在 DrawNodePropertiesImpl 后调用它
		std::function<void(class EditorGraph& graph, class EditorNode& node, const class Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx)> customDraw;
	};
}

