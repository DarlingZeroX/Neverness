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
#include "../../Core/Viewport.h"

namespace VisionGal
{
	class VG_ENGINE_API ViewportManager
	{
	public:
		ViewportManager();
		ViewportManager(const ViewportManager&) = delete;
		ViewportManager& operator=(const ViewportManager&) = delete;
		ViewportManager(ViewportManager&&) noexcept = default;
		ViewportManager& operator=(ViewportManager&&) noexcept = default;
		~ViewportManager() = default;

		static ViewportManager* Get();

		void SetMainViewport(Viewport* viewport);
		Viewport* GetMainViewport();
		Viewport* NewViewport(float2 size);
		void FrameUpdate();
	private:
		std::vector<Ref<Viewport>> m_Viewports;
		Viewport* m_MainViewport = nullptr;
	};
}
