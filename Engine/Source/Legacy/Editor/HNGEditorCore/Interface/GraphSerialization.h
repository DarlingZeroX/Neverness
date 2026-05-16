/*
* EditorGraph JSON 序列化 / 反序列化
*
* 🎯 目标：
* - 图可以保存 / 加载 / 分享
*
* 设计说明（务必阅读）：
* 1) 序列化内容
*   - nodes（包含 pins、properties、position）
*   - links
*
* 2) 不序列化的内容
*   - EditorGraph::context（运行期 ImNodeEditor 上下文指针）
*   - EditorGraph::registry（运行期 NodeRegistry 指针）
*   - dirty 等运行期状态可在加载后自行设置
*
* 3) ID 的保存方式
*   - ax::NodeEditor::NodeId / PinId / LinkId 本质可转为 int（通过 .Get()）
*   - 保存为整数，加载时用对应构造函数恢复
*
* 4) 版本字段
*   - 文件中会写入 "version"，后续结构变更时可做兼容迁移
*/

#pragma once

#include <string>
#include "../HNGEditorCoreConfig.h"
#include "EditorGraph.h"

namespace Horizon::NodeGraphEditor
{
	// 保存 EditorGraph 为 JSON 文件
	// 返回：
	// - true  : 保存成功
	// - false : 保存失败（文件不可写/路径无效/异常等）
	HNG_EDITOR_CORE_API bool SaveGraph(const EditorGraph& graph, const std::string& path);

	// 从 JSON 文件加载 EditorGraph
	// 说明：
	// - 若加载失败（文件不存在/解析失败），返回一个空图
	// - 加载后的 graph.context / graph.registry 需要上层自行重新绑定
	HNG_EDITOR_CORE_API EditorGraph LoadGraph(const std::string& path);
}

