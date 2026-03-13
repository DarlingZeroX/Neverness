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

#include "NodeGraphWindow.h"

namespace Horizon::NodeGraph
{
	NodeGraphWindow::NodeGraphWindow(const std::string& windowID)
		:m_WindowID(windowID)
	{
	}

	NodeGraphWindow::~NodeGraphWindow()
	{
	}

	void NodeGraphWindow::OnGUI()
	{
		IMNE::Begin(m_WindowID.c_str(), ImVec2(0.0f, 0.0f));
		
		IMNE::End();
	}
}
