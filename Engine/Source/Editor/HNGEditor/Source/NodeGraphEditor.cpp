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
#include "HNGEditorCore/Include/GraphCompiler.h"
#include <HNGRuntimeCore/Include/Core/Nodes.h>
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
		
		// 2. 添加节点（全部来自 NodeRegistry 元数据）
		// 注意：不能在多次 AddNode 之间长期持有对 nodes 元素的引用，
		// 否则 vector 可能在 push_back 时重新分配导致引用失效。
		// 这里采用“记录索引，再通过 editor.nodes[index] 访问”的方式避免悬空引用。

		const size_t entryIndex  = editor.nodes.size();
		editor.AddNode(NodeType::Entry);

		const size_t dlgIndex    = editor.nodes.size();
		editor.AddNode(NodeType::Dialogue);
		editor.nodes[dlgIndex].properties["text"] = "你好，欢迎来到 Demo!";

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

		RunDemo();

	}

	HNodeGraphEditor::~HNodeGraphEditor()
	{
	}

	void HNodeGraphEditor::DrawNodeGraphWindow()
	{
		RecompileIfDirty(m_EditorGraph);
		Horizon::NodeGraphRuntime::ExecuteGraph(m_RuntimeContext);
		Horizon::NodeGraphEditor::DrawEditorGraph(m_EditorGraph, &m_RuntimeContext);

		//IMNE::SetCurrentEditor(m_EditorContext);
		//IMNE::Begin("Test", ImVec2(0.0f, 0.0f));
		//IMNE::End();
		//IMNE::SetCurrentEditor(nullptr);
	}



}