/*
* Galgame 节点注册器：集中维护运行时(NodeRegistry) + 编辑器(NodeEditorRegistry)的注册逻辑。
*
* 目标：
* - 避免 NGEditorGalgame::RegisterGalgameNodes() 过于臃肿
* - 将“每个节点的 runtime 注册 + UI 注册”放在物理位置上紧挨着，方便维护
* - 按类别拆分（Core / Flow / Asset 等）
*/
#pragma once

#include "../VGEGExport.h"

#include <HNGRuntimeCore/Include/NodeRegistry.h>
#include <HNGEditorCore/Include/NodeEditorRegistry.h>

namespace VisionGal::Editor
{
	class VG_GALGAME_EDITOR_API GalgameNodeRegistry
	{
	public:
		static void RegisterAll(
			Horizon::NodeGraphRuntime::NodeRegistry& registry,
			Horizon::NodeGraphEditor::NodeEditorRegistry& editorRegistry);
	};
}

