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

#include "NodeGraphEditor.h"
#include "CommandInGraph.h"
#include "GraphCompiler.h"
#include <unordered_set>
#include <VGImgui/IncludeImGui.h>
#include <HNGEditorCore/Include/IMNEWrap.h>
#include <HCore/Interface/HConfig.h>

namespace Horizon::NodeGraph
{
	HNodeGraphEditor::HNodeGraphEditor()
	{
	}

	HNodeGraphEditor::~HNodeGraphEditor()
	{
	}
	void HNodeGraphEditor::Initialize(
		Horizon::NodeGraphEditor::EditorGraph* graph,
		Horizon::NodeGraphEditor::CommandManager* commandManager)
	{
		m_Graph = graph;
		m_CommandManager = commandManager;

		if (!m_Graph)
			return;

		// 绑定命令管理器：让 DrawEditorGraph 内部的删除/移动等操作走 Undo/Redo
		m_Graph->commandManager = m_CommandManager;

		// 为了让 DrawEditorGraph 可以工作，补齐默认 NodeEditor context
		if (m_Graph->context == nullptr)
		{
			m_Graph->context = MakeRef<Horizon::NodeGraphEditor::IMNEEditorContext>();
		}

		// runtime 初始状态
		m_IsPlaying = false;
		m_IsPaused = false;
		m_RuntimeGraph = Horizon::NodeGraphRuntime::RuntimeGraph{};
		m_RuntimeContext = Horizon::NodeGraphRuntime::RuntimeContext{};
		m_RuntimeContext.graph = nullptr;
	}

	void HNodeGraphEditor::HandleShortcuts()
	{
		if (!m_Graph)
			return;

		// Ctrl 相关快捷键：Copy / Paste（以及 Undo/Redo）
		// - Copy：只生成内存快照（不修改图状态）
		// - Paste：修改图状态，必须走 CommandSystem（Undo/Redo）
		ImGuiIO& io = ImGui::GetIO();
		const bool ctrlDown = io.KeyCtrl;

		// ----------------------------
		// Ctrl+C：Copy
		// ----------------------------
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_C, false))
		{
			m_CopyBuffer.nodes.clear();
			m_CopyBuffer.links.clear();

			if (m_Graph->selectedNodes.empty())
				return;

			std::unordered_set<ax::NodeEditor::PinId> pinsInSelection;

			for (const auto& node : m_Graph->nodes)
			{
				if (m_Graph->selectedNodes.find(node.id) == m_Graph->selectedNodes.end())
					continue;

				m_CopyBuffer.nodes.push_back(node);

				for (const auto& p : node.inputs)
					pinsInSelection.insert(p.id);
				for (const auto& p : node.outputs)
					pinsInSelection.insert(p.id);
			}

			// 复制“节点集合内部”的连线
			for (const auto& l : m_Graph->links)
			{
				if (pinsInSelection.find(l.startPinId) != pinsInSelection.end() &&
					pinsInSelection.find(l.endPinId) != pinsInSelection.end())
				{
					m_CopyBuffer.links.push_back(l);
				}
			}
		}

		// ----------------------------
		// Ctrl+V：Paste（走 CommandSystem）
		// ----------------------------
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_V, false))
		{
			if (!m_CommandManager || m_CopyBuffer.nodes.empty())
				return;

			const ImVec2 pasteOffset{ 40.0f, 40.0f };

			auto cmd = std::make_unique<Horizon::NodeGraphEditor::PasteNodesCommand>(
				*m_Graph,
				m_CopyBuffer,
				pasteOffset
			);

			auto* cmdPtr = cmd.get();
			m_CommandManager->ExecuteCommand(std::move(cmd));

			// 同步 selection：让后续拖拽（MultiMoveNodesCommand）工作正常
			ax::NodeEditor::ClearSelection();
			m_Graph->selectedNodes.clear();
			m_Graph->selectedLinks.clear();

			bool first = true;
			for (const auto& nodeId : cmdPtr->GetPastedNodeIds())
			{
				m_Graph->selectedNodes.insert(nodeId);
				ax::NodeEditor::SelectNode(nodeId, !first);
				first = false;
			}
		}

		// ----------------------------
		// Undo / Redo（Ctrl+Z / Ctrl+Y）
		// ----------------------------
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		{
			if (m_CommandManager) m_CommandManager->Undo();
		}
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_Y, false))
		{
			if (m_CommandManager) m_CommandManager->Redo();
		}
	}

	void HNodeGraphEditor::Draw()
	{
		if (!m_Graph)
			return;

		HandleShortcuts();
		// 传入 runtimeCtx 仅用于可视化高亮（执行路径/当前节点），不在此处执行运行时
		Horizon::NodeGraphEditor::DrawEditorGraph(*m_Graph, &m_RuntimeContext);
	}

	void HNodeGraphEditor::RecompileIfDirty()
	{
		using namespace Horizon::NodeGraphRuntime;

		if (!m_Graph)
			return;

		if (!m_Graph->dirty)
			return;

		// 必须由业务层提供 registry（NodeMeta/execute 等）
		if (!m_Graph->registry)
		{
			// 没有 registry 时无法编译，保持 dirty 以便业务层修复后可再次编译
			return;
		}

		m_RuntimeGraph = Horizon::NodeGraphEditor::Compile(*m_Graph, *m_Graph->registry);

		m_RuntimeContext = RuntimeContext{};
		m_RuntimeContext.graph = &m_RuntimeGraph;

		if (m_RuntimeGraph.entryNodeId != 0)
			m_RuntimeContext.execStack.push_back(m_RuntimeGraph.entryNodeId);

		m_Graph->dirty = false;
	}

	void HNodeGraphEditor::Update(float deltaTime)
	{
		using namespace Horizon::NodeGraphRuntime;

		// 业务层即使不播放，也可以读取 variables/高亮状态
		m_RuntimeContext.deltaTime = deltaTime;

		if (m_IsPlaying && !m_IsPaused)
		{
			RecompileIfDirty();
			ExecuteGraph(m_RuntimeContext);
		}
	}

	void HNodeGraphEditor::Play()
	{
		if (!m_Graph)
			return;

		m_IsPlaying = true;
		m_IsPaused = false;
		m_Graph->dirty = true; // 强制触发重新编译与重置 RuntimeContext
	}

	void HNodeGraphEditor::Pause()
	{
		m_IsPaused = true;
	}

	void HNodeGraphEditor::Stop()
	{
		m_IsPlaying = false;
		m_IsPaused = false;
		m_RuntimeContext = Horizon::NodeGraphRuntime::RuntimeContext{};
	}

	void HNodeGraphEditor::Recompile()
	{
		if (!m_Graph)
			return;
		m_Graph->dirty = true;
	}

	Horizon::NodeGraphRuntime::Value* HNodeGraphEditor::TryGetVariable(const std::string& name)
	{
		auto it = m_RuntimeContext.variables.find(name);
		if (it == m_RuntimeContext.variables.end())
			return nullptr;
		return &it->second;
	}
}