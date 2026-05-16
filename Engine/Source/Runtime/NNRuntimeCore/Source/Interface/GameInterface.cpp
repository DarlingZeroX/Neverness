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

#include "GameInterface.h"

namespace VisionGal
{
	ICamera::~ICamera()
	{
		for (auto viewport: m_Viewports)
		{
			viewport->RemoveCamera(this);
		}
	}

	void ICamera::AttachViewport(IViewport* viewport)
	{
		m_Viewports.push_back(viewport);
	}
}
