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

#include "Sequence/ComponentDrawerRegistry.h"
#include "Sequence/ComponentDrawers.h"

namespace VisionGal::Editor
{
	GalSeqComDrawerRegistry::GalSeqComDrawerRegistry()
	{
		//RegisterDrawer(MakeRef<GSCD_CommonDialogue>());
		//RegisterDrawer(MakeRef<GSCD_ChangeFigure>());
		//RegisterDrawer(MakeRef<GSCD_ChangeBackground>());
	}

	bool GalSeqComDrawerRegistry::RegisterDrawer(const Ref<IGalSeqComDrawer>& drawer)
	{
		m_Drawers[drawer->GetBindType()] = drawer;

		return true;
	}

	IGalSeqComDrawer* GalSeqComDrawerRegistry::GetDrawer(const std::string& type)
	{
		if (m_Drawers.find(type) != m_Drawers.end())
		{
			return m_Drawers[type].get();
		}

		return nullptr;
	}

	GalSeqComDrawerRegistry& GalSeqComDrawerRegistry::GetInstance()
	{
		static GalSeqComDrawerRegistry s_Instance;
		return s_Instance;
	}
}
