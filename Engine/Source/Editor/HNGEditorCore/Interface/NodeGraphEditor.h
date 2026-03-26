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
#include "../HNGEditorCoreConfig.h"
#include "EditorGraph.h"
#include "../Include/CommandSystem.h"
#include "../Include/CommandInGraph.h"

#include <string>
#include <HNGRuntimeCore/Include/RuntimeContext.h>
#include <HNGRuntimeCore/Include/RuntimeGraph.h>

namespace Horizon::NodeGraph
{
	class HNG_EDITOR_CORE_API HNodeGraphEditor
	{
	public:
		HNodeGraphEditor();
		~HNodeGraphEditor();

		// 初始化（由外部传入图与命令系统）
		// - 图的 registry/editorRegistry 由业务层注册完成
		// - commandManager 用于 Undo/Redo 与 Paste/Delete 等命令执行
		void Initialize(
			Horizon::NodeGraphEditor::EditorGraph* graph,
			Horizon::NodeGraphEditor::CommandManager* commandManager);

		// 绘制：仅处理快捷键并调用通用 DrawEditorGraph
		void Draw();

		// ----------------------------
		// Runtime：编译 + 执行（业务层无需重复实现）
		// ----------------------------
		void Update(float deltaTime);

		void Play();
		void Pause();
		void Stop();
		void Recompile();

		bool IsPlaying() const { return m_IsPlaying; }
		bool IsPaused() const { return m_IsPaused; }

		Horizon::NodeGraphRuntime::RuntimeContext& GetRuntimeContext() { return m_RuntimeContext; }

		// 加分项：方便 UI 读取变量
		Horizon::NodeGraphRuntime::Value* TryGetVariable(const std::string& name);

	private:
		Horizon::NodeGraphEditor::EditorGraph* m_Graph = nullptr;
		Horizon::NodeGraphEditor::CommandManager* m_CommandManager = nullptr;

		// Copy/Paste：最近一次复制得到的节点/连线快照
		// - Copy 本身不修改图状态
		// - Paste 必须走 CommandSystem（Undo/Redo）
		Horizon::NodeGraphEditor::NodeGraphCopyBuffer m_CopyBuffer;

		// ----------------------------
		// Runtime 数据（由 EditorCore 管理生命周期）
		// ----------------------------
		Horizon::NodeGraphRuntime::RuntimeGraph m_RuntimeGraph;
		Horizon::NodeGraphRuntime::RuntimeContext m_RuntimeContext;

		bool m_IsPlaying = false;
		bool m_IsPaused = false;

		// Ctrl+C / Ctrl+V / Ctrl+Z / Ctrl+Y
		void HandleShortcuts();

		// 编译：Graph -> RuntimeGraph（仅在 dirty 时进行）
		void RecompileIfDirty();
	};

}