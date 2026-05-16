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

#include "Engine/Manager/ViewportManager.h"

namespace VisionGal
{
	ViewportManager::ViewportManager()
	{
	}

	Viewport* ViewportManager::GetMainViewport()
	{
		return m_MainViewport;
	}

	ViewportManager* ViewportManager::Get()
	{
		static ViewportManager manager;
		return &manager;
	}

	void ViewportManager::SetMainViewport(Viewport* viewport)
	{
		m_MainViewport = viewport;
	}

	Viewport* ViewportManager::NewViewport(float2 size)
	{
		auto viewport = MakeRef<Viewport>(size);

		m_Viewports.push_back(viewport);

		return viewport.get();
	}

	void ViewportManager::FrameUpdate()
	{
		for (auto& viewport : m_Viewports)
		{
			viewport->FrameUpdate();
		}
	}
}


