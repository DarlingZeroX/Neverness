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
#include "../Config.h"
#include "IComponentDrawer.h"
#include <memory>
#include <unordered_map>

namespace VisionGal::Editor
{
	class VG_EDITOR_FRAMEWORK_API ComponentDrawerRegistry
	{
		ComponentDrawerRegistry() = default;
	public:
		ComponentDrawerRegistry(const ComponentDrawerRegistry&) = default;
		ComponentDrawerRegistry& operator=(const ComponentDrawerRegistry&) = default;
		ComponentDrawerRegistry(ComponentDrawerRegistry&&) noexcept = default;
		ComponentDrawerRegistry& operator=(ComponentDrawerRegistry&&) noexcept = default;
		~ComponentDrawerRegistry() = default;

		bool RegisterDrawer(const Ref<IComponentDrawer>& drawer);

		IComponentDrawer* GetDrawer(const String& type);

		static ComponentDrawerRegistry& GetInstance();
	private:
		std::unordered_map<String,Ref< IComponentDrawer >> m_Drawers;
	};
}
