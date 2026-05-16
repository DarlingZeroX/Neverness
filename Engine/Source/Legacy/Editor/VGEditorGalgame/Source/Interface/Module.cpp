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

#include "VGEditorGalgame/Module.h"

#include <NNRuntimeImGui/IncludeImGui.h>

#include <algorithm>

#include <VGEditorGalgame/Interface/GalgameNodeRegistry.h>
#include "DialogueListEditorPanel.h"

#include <VGGalgameNodeGraph/Include/VGNodeExec_Galgame.h>

namespace VisionGal::Editor
{
	using namespace Horizon::NodeGraphRuntime;
	using namespace Horizon::NodeGraphEditor;
	//using namespace VisionGal::Runtime;

	// Galgame 节点：使用 NodeRegistry 的动态类型注册（TypeId + 注册系统）。

	VisualGalgame::VisualGalgame() = default;
	VisualGalgame::~VisualGalgame()
	{
		// 清理业务侧面板桥接，避免静态指针悬挂
		if (m_DialogueEditor && GetDialogueListEditorPanelInstance() == m_DialogueEditor.get())
			SetDialogueListEditorPanelInstance(nullptr);
	}

	void VisualGalgame::Initialize()
	{
		// 复杂节点面板（业务层 UI）
		m_DialogueEditor = std::make_unique<DialogueListEditorPanel>();
		SetDialogueListEditorPanelInstance(m_DialogueEditor.get());

		// 绑定图数据：EditorGraph -> NodeRegistry/NodeEditorRegistry/CommandManager
		m_Graph.context = MakeRef<IMNEEditorContext>();
		m_Graph.registry = &m_Registry;
		m_Graph.editorRegistry = &m_NodeEditorRegistry;
		m_Graph.commandManager = &m_CommandManager;

		// 初始化通用编辑器（编辑 + 编译 + 运行时执行核心）
		m_CoreEditor.Initialize(&m_Graph, &m_CommandManager);

		RegisterGalgameNodes();
		SetupInitialGraph();
	}

	void VisualGalgame::RegisterGalgameNodes()
	{
		GalgameNodeRegistry::RegisterAll(m_Registry, m_NodeEditorRegistry);
	}

	void VisualGalgame::SetupInitialGraph()
	{
		m_Graph.nodes.clear();
		m_Graph.links.clear();
		m_Graph.selectedNodes.clear();
		m_Graph.selectedLinks.clear();
		m_Graph.nodeIndexById.clear();
		m_Graph.pinOwnerById.clear();

		m_Graph.idGen.Reset(m_Graph.idGen.GetState());

		const ax::NodeEditor::NodeId entryId = m_Graph.AddNode(m_Registry.FindType("Entry")).id;
		const ax::NodeEditor::NodeId dlgId = m_Graph.AddNode(m_Registry.FindType("DialogueList")).id;

		auto* entryNode = m_Graph.FindNode(entryId);
		auto* dlgNode = m_Graph.FindNode(dlgId);
		if (!entryNode || !dlgNode)
			return;

		// 设置初始位置（仅 UI）
		entryNode->position = ImVec2(0.0f, 0.0f);
		dlgNode->position = ImVec2(250.0f, 0.0f);

		// 创建连线 Entry.Next -> DialogueList.In
		if (entryNode->outputs.empty() || dlgNode->inputs.empty())
			return;

		EditorLink link;
		link.id = m_Graph.idGen.NewLinkId();
		link.startPinId = entryNode->outputs.front().id;
		link.endPinId = dlgNode->inputs.front().id;
		m_Graph.links.push_back(link);

		m_Graph.dirty = true;
		m_Graph.RebuildIndices();
	}

	void VisualGalgame::DrawToolbar()
	{
		ImGui::Separator();
		ImGui::TextUnformatted("Galgame Toolbar");

		if (ImGui::Button("Play")) m_CoreEditor.Play();
		ImGui::SameLine();
		if (ImGui::Button("Pause")) m_CoreEditor.Pause();
		ImGui::SameLine();
		if (ImGui::Button("Stop")) m_CoreEditor.Stop();
		ImGui::SameLine();
		if (ImGui::Button("Recompile")) m_CoreEditor.Recompile();
	}

	void VisualGalgame::DrawPreviewWindow()
	{
		ImGui::Separator();
		ImGui::Begin("Preview");

		const Value* speakerV = m_CoreEditor.TryGetVariable(VisionGal::Runtime::Vars::CurrentSpeaker);
		const Value* textV = m_CoreEditor.TryGetVariable(VisionGal::Runtime::Vars::CurrentText);

		const char* speaker = (speakerV && speakerV->type == ValueType::String) ? speakerV->AsString().c_str() : "角色名";
		const char* text = (textV && textV->type == ValueType::String) ? textV->AsString().c_str() : "";

		ImGui::TextUnformatted(speaker);
		ImGui::Spacing();
		ImGui::BeginChild("PreviewTextChild", ImVec2(0, 120.0f), true);
		ImGui::TextWrapped("%s", text);
		ImGui::EndChild();

		ImGui::Spacing();

		const bool canAdvance = m_CoreEditor.IsPlaying() && !m_CoreEditor.IsPaused();
		ImGui::BeginDisabled(!canAdvance);
		if (ImGui::Button("点击继续")) m_CoreEditor.GetRuntimeContext().variables["Next"] = Value::FromBool(true);
		ImGui::EndDisabled();

		ImGui::End();
	}

	void VisualGalgame::DrawEditorWindow()
	{
		ImGui::Begin("Galgame NodeGraph Editor");

		DrawToolbar();

		ImGui::End();

		m_CoreEditor.Draw();

		// 独立窗口：复杂节点面板（例如 DialogueList Editor）
		if (m_DialogueEditor)
			m_DialogueEditor->Draw(m_Graph);

		DrawPreviewWindow();
	}

	void VisualGalgame::Update()
	{
		Update(0.016f);
	}

	void VisualGalgame::Update(float deltaTime)
	{
		// 运行时编译/执行由 CoreEditor 统一处理
		m_CoreEditor.Update(deltaTime);
	}

	void VisualGalgame::OpenDialogueEditor(ax::NodeEditor::NodeId nodeId)
	{
		// 预留：独立对白编辑窗口
		// 当前实现为最小可用占位（便于后续扩展 speaker/text 结构化编辑）
		(void)nodeId;
	}
}
