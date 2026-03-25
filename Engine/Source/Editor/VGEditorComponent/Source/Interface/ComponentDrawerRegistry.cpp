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

#include "ComponentDrawerRegistry.h"

namespace VisionGal::Editor
{

	class EmptyComponentDrawer : public IComponentDrawer
	{
	public:
		void OnGUI(IEntity* entity) override
		{
		}
		const String GetBindType() const override
		{
			return "";
		}
	};

	bool ComponentDrawerRegistry::RegisterDrawer(const Ref<IComponentDrawer>& drawer)
	{
		m_Drawers[drawer->GetBindType()] = drawer;

		return true;
	}

	IComponentDrawer* ComponentDrawerRegistry::GetDrawer(const String& type)
	{
		static EmptyComponentDrawer s_EmptyDrawer;

		if (m_Drawers.find(type) != m_Drawers.end())
		{
			return m_Drawers[type].get();
		}

		return &s_EmptyDrawer;
	}

	ComponentDrawerRegistry& ComponentDrawerRegistry::GetInstance()
	{
		static ComponentDrawerRegistry s_Instance;
		return s_Instance;
	}
}
