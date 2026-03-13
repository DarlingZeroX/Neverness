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
#include <string>
#include "Config.h"

namespace Horizon::NodeGraph
{
	class H_NODE_GRAPH_EDITOR_API NodeGraphWindow
	{
	public:
		NodeGraphWindow(const std::string& windowID);
		~NodeGraphWindow();

		virtual void OnGUI();
	private:
		std::string m_WindowID;
	};

}