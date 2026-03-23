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
#include "HNGEditorCore/Include/NodeFactory.h"
#include "HNGEditorCore/Interface/GraphCompiler.h"
#include <HNGRuntimeCore/Include/Core/Nodes.h>
#include <VGImgui/IncludeImGui.h>
#include <cstdio>
#include <utility>

namespace Horizon::NodeGraph
{
	Horizon::NodeGraphRuntime::RuntimeGraph HNodeGraphEditor::RecompileIfDirty(NodeGraphEditor::EditorGraph& editor)
	{
		using namespace Horizon::NodeGraphRuntime;

		if (!editor.dirty)
		{
			return m_RuntimeGraph;
		}

		m_RuntimeGraph = Compile(editor, m_Registry);
		editor.dirty = false;

		m_RuntimeContext = RuntimeContext{};
		m_RuntimeContext.graph = &m_RuntimeGraph;
		if (m_RuntimeGraph.entryNodeId != 0)
		{
			m_RuntimeContext.execStack.push_back(m_RuntimeGraph.entryNodeId);
		}

		return m_RuntimeGraph;
	}

	void HNodeGraphEditor::RunDemo()
	{
		using namespace Horizon::NodeGraphEditor;
		using namespace Horizon::NodeGraphRuntime;
		
		// 1. 创建 EditorGraph
		EditorGraph& editor = m_EditorGraph;
		editor.context = CreateRef<Horizon::NodeGraphEditor::IMNEEditorContext>();
		editor.registry = &m_Registry;
		editor.editorRegistry = &m_NodeEditorRegistry;
		
		// 2. 添加节点（全部来自 NodeRegistry 元数据）
		// 注意：不能在多次 AddNode 之间长期持有对 nodes 元素的引用，
		// 否则 vector 可能在 push_back 时重新分配导致引用失效。
		// 这里采用“记录索引，再通过 editor.nodes[index] 访问”的方式避免悬空引用。

		const size_t entryIndex  = editor.nodes.size();
		editor.AddNode(NodeType::Entry);

		const size_t dlgIndex    = editor.nodes.size();
		editor.AddNode(NodeType::Dialogue);
		editor.nodes[dlgIndex].GetProperty("text") = Value::FromString("你好，欢迎来到 Demo!");

		const size_t branchIndex = editor.nodes.size();
		editor.AddNode(NodeType::Branch);
		
		// 3. 连接 pins
		EditorLink link1;
		link1.id = ax::NodeEditor::LinkId(1);
		link1.startPinId = editor.nodes[entryIndex].outputs[0].id;
		link1.endPinId   = editor.nodes[dlgIndex].inputs[0].id;
		editor.links.push_back(link1);
		
		EditorLink link2;
		link2.id = ax::NodeEditor::LinkId(2);
		link2.startPinId = editor.nodes[dlgIndex].outputs[0].id;
		link2.endPinId   = editor.nodes[branchIndex].inputs[0].id;
		editor.links.push_back(link2);
		
		// Branch True/False 输出未连接到其它节点（可扩展）

		editor.dirty = true;
	}

	HNodeGraphEditor::HNodeGraphEditor()
	{
		//m_EditorGraph.context = CreateRef<NodeGraphEditor::IMNEEditorContext>();
		//m_EditorGraph.nodes.push_back(NodeGraphEditor::CreateEntryNode());
		//m_EditorGraph.nodes.push_back(NodeGraphEditor::CreateDialogueNode());
		//m_EditorGraph.nodes.push_back(NodeGraphEditor::CreateBranchNode());
		using namespace Horizon::NodeGraphRuntime;
		m_Registry.Register(NodeMeta{
			NodeType::Entry,
			"Entry",
			{},
			{
				{ "Next", SlotType::Exec, false }
			},
			EntryNodeExecute
		});

		m_Registry.Register(NodeMeta{
			NodeType::Dialogue,
			"Dialogue",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Next", SlotType::Exec, false },
				{ "Text", SlotType::String, false }
			},
			DialogueNodeExecute
		});

		// 旧 Branch 节点保留，供需要 Bool 条件输入的图使用
		m_Registry.Register(NodeMeta{
			NodeType::Branch,
			"Branch",
			{
				{ "In", SlotType::Exec, true },
				{ "Condition", SlotType::Bool, true }
			},
			{
				{ "True", SlotType::Exec, false },
				{ "False", SlotType::Exec, false }
			},
			BranchNodeExecute
		});

		m_Registry.Register(NodeMeta{
			NodeType::Delay,
			"Delay",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Next", SlotType::Exec, false }
			},
			DelayNodeExecute
		});

		// SetVariable 节点：执行表达式并写入变量表
		m_Registry.Register(NodeMeta{
			NodeType::SetVariable,
			"SetVariable",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Next",       SlotType::Exec,   false },
				{ "Name",       SlotType::String, false }, // 变量名
				{ "Expression", SlotType::String, false }  // 表达式源码
			},
			SetVariableNodeExecute
		});

		// GetVariable 节点：从变量表读取变量值并输出
		m_Registry.Register(NodeMeta{
			NodeType::GetVariable,
			"GetVariable",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "Next", SlotType::Exec,   false },
				{ "Name", SlotType::String, false }, // 变量名
				{ "Value",SlotType::String, false }  // 变量值（实际 Value 类型由运行时决定）
			},
			GetVariableNodeExecute
		});

		// Condition 节点：基于表达式的条件分支
		m_Registry.Register(NodeMeta{
			NodeType::Condition,
			"Condition",
			{
				{ "In", SlotType::Exec, true }
			},
			{
				{ "True",      SlotType::Exec,   false },
				{ "False",     SlotType::Exec,   false },
				{ "Condition", SlotType::String, false } // 条件表达式
			},
			ConditionNodeExecute
		});

		// ------------------------------------------------------------------
		// Editor 专用：注册 NodeEditorMeta / PropertyMeta
		// - 注意：与 Runtime NodeRegistry 完全独立
		// - 这里为 Dialogue / SetVariable（以及为保持旧 UI 兼容，额外注册 GetVariable/Condition）
		//   提供属性默认值与 ImGui 控件类型
		// ------------------------------------------------------------------
		{
			using Horizon::NodeGraphEditor::NodeEditorMeta;
			using Horizon::NodeGraphEditor::PropertyMeta;
			using Horizon::NodeGraphEditor::PropertyWidgetType;

			// Dialogue
			m_NodeEditorRegistry.Register(NodeEditorMeta{
				NodeType::Dialogue,
				"Dialogue",
				"Dialogue",
				{
					PropertyMeta{
						"text",
						"Text",
						ValueType::String,
						PropertyWidgetType::MultilineText,
						Value::FromString(""),
						{}
					},
					PropertyMeta{
						"speaker",
						"Speaker",
						ValueType::String,
						PropertyWidgetType::InputText,
						Value::FromString(""),
						{}
					}
				}
			});

			// SetVariable
			m_NodeEditorRegistry.Register(NodeEditorMeta{
				NodeType::SetVariable,
				"SetVariable",
				"Variable",
				{
					PropertyMeta{
						"name",
						"Name",
						ValueType::String,
						PropertyWidgetType::InputText,
						Value::FromString(""),
						{}
					},
					PropertyMeta{
						"value",
						"Expression",
						ValueType::String, // Runtime 的 Expression 槽当前要求 String
						PropertyWidgetType::InputText,
						Value::FromString(""),
						{}
					}
				}
			});

			// GetVariable（为保持旧功能：编辑变量名）
			m_NodeEditorRegistry.Register(NodeEditorMeta{
				NodeType::GetVariable,
				"GetVariable",
				"Variable",
				{
					PropertyMeta{
						"name",
						"Name",
						ValueType::String,
						PropertyWidgetType::InputText,
						Value::FromString(""),
						{}
					}
				}
			});

			// Condition（为保持旧功能：编辑条件表达式）
			m_NodeEditorRegistry.Register(NodeEditorMeta{
				NodeType::Condition,
				"Condition",
				"Logic",
				{
					PropertyMeta{
						"condition",
						"Condition",
						ValueType::String,
						PropertyWidgetType::InputText,
						Value::FromString(""),
						{}
					}
				}
			});
		}

		// 绑定命令管理器到 EditorGraph，启用命令驱动的编辑操作
		m_EditorGraph.commandManager = &m_CommandManager;

		RunDemo();

	}

	HNodeGraphEditor::~HNodeGraphEditor()
	{
	}

	void ShowStyleEditor(bool* show = nullptr)
	{
		namespace ed = ax::NodeEditor;

		if (!ImGui::Begin("Style", show))
		{
			ImGui::End();
			return;
		}

		auto paneWidth = ImGui::GetContentRegionAvail().x;

		auto& editorStyle = ed::GetStyle();
		//ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
		ImGui::TextUnformatted("Values");
		//ImGui::Spring();
		if (ImGui::Button("Reset to defaults"))
			editorStyle = ed::Style();
		//ImGui::EndHorizontal();
		ImGui::Spacing();
		ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
		ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
		ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
		ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
		ImGui::DragFloat("Hovered Node Border Offset", &editorStyle.HoverNodeBorderOffset, 0.1f, -40.0f, 40.0f);
		ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
		ImGui::DragFloat("Selected Node Border Offset", &editorStyle.SelectedNodeBorderOffset, 0.1f, -40.0f, 40.0f);
		ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
		ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
		ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
		//ImVec2  SourceDirection;
		//ImVec2  TargetDirection;
		ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
		ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
		ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
		ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
		//ImVec2  PivotAlignment;
		//ImVec2  PivotSize;
		//ImVec2  PivotScale;
		//float   PinCorners;
		//float   PinRadius;
		//float   PinArrowSize;
		//float   PinArrowWidth;
		ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
		ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

		ImGui::Separator();

		static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
		//ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
		ImGui::TextUnformatted("Filter Colors");
		//ImGui::Spring();
		ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_DisplayRGB);
		//ImGui::Spring(0);
		ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_DisplayHSV);
		//ImGui::Spring(0);
		ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_DisplayHex);
		//ImGui::EndHorizontal();
		static ImGuiTextFilter filter;
		filter.Draw("##filter", paneWidth);

		ImGui::Spacing();

		ImGui::PushItemWidth(-160);
		for (int i = 0; i < ed::StyleColor_Count; ++i)
		{
			auto name = ed::GetStyleColorName((ed::StyleColor)i);
			if (!filter.PassFilter(name))
				continue;

			ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
		}
		ImGui::PopItemWidth();

		ImGui::End();
	}

	void HNodeGraphEditor::DrawNodeGraphWindow()
	{
		// 处理 Undo / Redo 快捷键（Ctrl+Z / Ctrl+Y）
		ImGuiIO& io = ImGui::GetIO();
		const bool ctrlDown = io.KeyCtrl;
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_Z, false))
		{
			m_CommandManager.Undo();
		}
		if (ctrlDown && ImGui::IsKeyPressed(ImGuiKey_Y, false))
		{
			m_CommandManager.Redo();
		}

		RecompileIfDirty(m_EditorGraph);
		Horizon::NodeGraphRuntime::ExecuteGraph(m_RuntimeContext);
		Horizon::NodeGraphEditor::DrawEditorGraph(m_EditorGraph, &m_RuntimeContext);

		//IMNE::SetCurrentEditor(m_EditorContext);
		//IMNE::Begin("Test", ImVec2(0.0f, 0.0f));
		//IMNE::End();
		//IMNE::SetCurrentEditor(nullptr);

		//ImGui::ShowDemoWindow();
		//ShowStyleEditor();
	}



}