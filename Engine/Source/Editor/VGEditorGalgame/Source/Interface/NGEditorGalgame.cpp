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

#include "NGEditorGalgame.h"

#include <VGImgui/IncludeImGui.h>

#include <algorithm>

#include <VGGalgameRuntime/Include/VGNodeExec_Galgame.h>

namespace VisionGal::Editor
{
	using namespace Horizon::NodeGraphRuntime;
	using namespace Horizon::NodeGraphEditor;
	using namespace VisionGal::Runtime;

	// Galgame 节点映射到现有 NodeType（不修改 Core 枚举/编译器）。
	static constexpr NodeType NODETYPE_DialogueList = NodeType::Dialogue;
	static constexpr NodeType NODETYPE_Choice = NodeType::Branch;
	static constexpr NodeType NODETYPE_ShowCharacter = NodeType::SetVariable;
	static constexpr NodeType NODETYPE_PlayBGM = NodeType::Delay;
	static constexpr NodeType NODETYPE_SetBackground = NodeType::Condition;
	static constexpr const char* PIN_Text = "Text"; // DialogueList -> Text

	NGEditorGalgame::NGEditorGalgame() = default;
	NGEditorGalgame::~NGEditorGalgame() = default;

	void NGEditorGalgame::Initialize()
	{
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

	void NGEditorGalgame::RegisterGalgameNodes()
	{
		m_Registry.Register(NodeMeta{
			NodeType::Entry,
			"Entry",
			{},
			{
				{ "Next", SlotType::Exec, false }
			},
			VisionGal::Runtime::EntryExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NodeType::Entry,
			"Entry",
			"Core",
			{}
		});

		m_Registry.Register(NodeMeta{
			NODETYPE_DialogueList,
			"DialogueList",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Next", SlotType::Exec, false },
				{ PIN_Text, SlotType::String, false }
			},
			VisionGal::Runtime::DialogueListExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NODETYPE_DialogueList,
			"DialogueList",
			"Galgame",
			{
				PropertyMeta{ "text", "dialogueList", ValueType::String, PropertyWidgetType::MultilineText, Value::FromString(""), {} }
			}
		});

		m_Registry.Register(NodeMeta{
			NODETYPE_Choice,
			"Choice",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Option1", SlotType::Exec, false },
				{ "Option2", SlotType::Exec, false }
			},
			VisionGal::Runtime::ChoiceExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NODETYPE_Choice,
			"Choice",
			"Galgame",
			{}
		});

		m_Registry.Register(NodeMeta{
			NODETYPE_ShowCharacter,
			"ShowCharacter",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Out", SlotType::Exec, false },
				{ "Name", SlotType::String, false },
				{ "Expression", SlotType::String, false },
				{ "Position", SlotType::String, false }
			},
			VisionGal::Runtime::ShowCharacterExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NODETYPE_ShowCharacter,
			"ShowCharacter",
			"Galgame",
			{
				PropertyMeta{ "name", "name", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} },
				PropertyMeta{ "value", "expression", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} },
				PropertyMeta{ "position", "position", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});

		m_Registry.Register(NodeMeta{
			NODETYPE_PlayBGM,
			"PlayBGM",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Out", SlotType::Exec, false },
				{ "BgmName", SlotType::String, false }
			},
			VisionGal::Runtime::PlayBGMExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NODETYPE_PlayBGM,
			"PlayBGM",
			"Galgame",
			{
				PropertyMeta{ "bgmName", "bgmName", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});

		m_Registry.Register(NodeMeta{
			NODETYPE_SetBackground,
			"SetBackground",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Out", SlotType::Exec, false },
				{ "BackgroundName", SlotType::String, false }
			},
			VisionGal::Runtime::SetBackgroundExecute
		});

		m_NodeEditorRegistry.Register(NodeEditorMeta{
			NODETYPE_SetBackground,
			"SetBackground",
			"Galgame",
			{
				PropertyMeta{ "backgroundName", "backgroundName", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});
	}

	void NGEditorGalgame::SetupInitialGraph()
	{
		m_Graph.nodes.clear();
		m_Graph.links.clear();
		m_Graph.selectedNodes.clear();
		m_Graph.selectedLinks.clear();
		m_Graph.nodeIndexById.clear();
		m_Graph.pinOwnerById.clear();

		m_Graph.idGen.Reset(m_Graph.idGen.GetState());

		const ax::NodeEditor::NodeId entryId = m_Graph.AddNode(NodeType::Entry).id;
		const ax::NodeEditor::NodeId dlgId = m_Graph.AddNode(NODETYPE_DialogueList).id;

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

	void NGEditorGalgame::DrawToolbar()
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

	void NGEditorGalgame::DrawPreviewWindow()
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

	void NGEditorGalgame::DrawEditorWindow()
	{
		ImGui::Begin("Galgame NodeGraph Editor");

		DrawToolbar();

		ImGui::End();

		m_CoreEditor.Draw();

		DrawPreviewWindow();
	}

	void NGEditorGalgame::Update()
	{
		Update(0.016f);
	}

	void NGEditorGalgame::Update(float deltaTime)
	{
		// 运行时编译/执行由 CoreEditor 统一处理
		m_CoreEditor.Update(deltaTime);
	}

	void NGEditorGalgame::OpenDialogueEditor(ax::NodeEditor::NodeId nodeId)
	{
		// 预留：独立对白编辑窗口
		// 当前实现为最小可用占位（便于后续扩展 speaker/text 结构化编辑）
		(void)nodeId;
	}
}
