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
#include "Interface.h"
#include <memory>
#include <unordered_map>

namespace VisionGal::Editor
{
	class ComponentDrawerManager
	{
	public:
		ComponentDrawerManager() = default;
		ComponentDrawerManager(const ComponentDrawerManager&) = default;
		ComponentDrawerManager& operator=(const ComponentDrawerManager&) = default;
		ComponentDrawerManager(ComponentDrawerManager&&) noexcept = default;
		ComponentDrawerManager& operator=(ComponentDrawerManager&&) noexcept = default;
		~ComponentDrawerManager() = default;

		bool RegisterDrawer(const Ref<IComponentDrawer>& drawer);

		IComponentDrawer* GetDrawer(const String& type);

	private:
		std::unordered_map<String,Ref< IComponentDrawer >> m_Drawers;
	};
}
