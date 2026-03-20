/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*/

#pragma once

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
		NodeGraphRuntime::NodeType type;

		// UI 展示
		std::string displayName;
		std::string category;

		// 属性描述
		std::vector<PropertyMeta> properties;
	};
}

