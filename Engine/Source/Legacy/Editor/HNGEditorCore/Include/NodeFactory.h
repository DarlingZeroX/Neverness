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
#include "../Interface/NodeEditorCore.h"
#include "../Interface/EditorGraph.h"
#include "GraphIdGenerator.h"
#include <NNNodeGraphCore/Include/NodeMeta.h>

namespace Horizon::NodeGraphEditor
{
	// ------------------------------------------------------------
	// CreateNodeFromMeta：EditorNode 的唯一创建入口（数据驱动）
	//
	// 设计目标：
	// - Editor 不再手写 inputs/outputs pins（避免重复、易错、难维护）
	// - 节点结构完全由 Runtime 的 NodeMeta 描述（真正“蓝图编辑器”）
	//
	// 行为说明：
	// - 自动生成：
	//   - inputs / outputs
	//   - pin name
	//   - slot type
	//   - isInput
	// - pins 的 id / node.id 由 graph.idGen 生成
	//
	// 约束：
	// - meta.inputs / meta.outputs 各自代表输入/输出 pin 列表；
	//   SlotMeta::isInput 字段在这里作为一致性校验来源（不依赖它决定方向）。
	// ------------------------------------------------------------
	EditorNode CreateNodeFromMeta(
		const Horizon::NodeGraphRuntime::NodeMeta& meta,
		GraphIdGenerator& idGen
	);
}