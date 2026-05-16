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

#include "IMNEWrap.h"

namespace Horizon::NodeGraphEditor
{
	namespace IMNE = ax::NodeEditor;

	IMNEEditorContext::IMNEEditorContext()
		:m_EditorContext(nullptr)
	{
		IMNE::Config config;
		m_EditorContext = IMNE::CreateEditor(&config);
	}

	IMNEEditorContext::IMNEEditorContext(const ax::NodeEditor::Config& config)
	{
		m_EditorContext = IMNE::CreateEditor(&config);
	}

	IMNEEditorContext::~IMNEEditorContext()
	{
		if (m_EditorContext != nullptr)
		{
			IMNE::DestroyEditor(m_EditorContext);
		}

		m_EditorContext = nullptr;
	}

	void IMNEEditorContext::SetContext()
	{
		IMNE::SetCurrentEditor(m_EditorContext);
	}

	void IMNEEditorContext::ResetContext()
	{
		IMNE::SetCurrentEditor(nullptr);
	}
}
