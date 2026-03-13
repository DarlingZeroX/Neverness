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

namespace Horizon::NodeGraph
{
	NodeGraphEditor::NodeGraphEditor()
		:m_EditorContext(nullptr)
	{
		Initialize();
	}

	NodeGraphEditor::~NodeGraphEditor()
	{
		if (m_EditorContext != nullptr)
		{
			IMNE::DestroyEditor(m_EditorContext);
		}

		m_EditorContext = nullptr;
	}

	void NodeGraphEditor::Initialize()
	{
		if (m_EditorContext == nullptr)
		{
			IMNE::Config config;
			m_EditorContext = IMNE::CreateEditor(&config);

			//ImNodeEditor::Style style;
		}
	}

	void NodeGraphEditor::SetContext()
	{
		IMNE::SetCurrentEditor(m_EditorContext);
	}

	void NodeGraphEditor::ResetContext()
	{
		IMNE::SetCurrentEditor(nullptr);
	}

	void NodeGraphEditor::DrawNodeGraphWindow(NodeGraphWindow& window)
	{
		IMNE::SetCurrentEditor(m_EditorContext);
	
		window.OnGUI();
	
		IMNE::SetCurrentEditor(nullptr);
	}

}