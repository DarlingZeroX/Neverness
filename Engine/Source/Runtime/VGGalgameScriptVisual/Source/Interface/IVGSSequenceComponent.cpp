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

#include "IVGSSequenceComponent.h"
#include "VisualSequence/SequenceComponents.h"

namespace VisionGal
{
	IVGSSequenceComponentManager::IVGSSequenceComponentManager()
	{
		//RegisteredComponents.emplace(VGSSC_CommonDialogue::StaticGetTypeNameID(), MakeRef<VGSSC_CommonDialogue>());
		//RegisteredComponents.emplace(VGSSC_ChangeFigure::StaticGetTypeNameID(), MakeRef<VGSSC_ChangeFigure>());
		////RegisteredComponents.emplace(VGSSC_ShowPicture::StaticGetTypeNameID(), MakeRef<VGSSC_ShowPicture>());
		//RegisteredComponents.emplace(VGSSC_ChangeBackground::StaticGetTypeNameID(), MakeRef<VGSSC_ChangeBackground>());

		EmplaceComponentType<VGSSC_CommonDialogue>();
		EmplaceComponentType<VGSSC_ChangeFigure>();
		EmplaceComponentType<VGSSC_ChangeBackground>();
	}

	IVGSSequenceComponentManager& IVGSSequenceComponentManager::Get()
	{
		static IVGSSequenceComponentManager instance;
		return instance;
	}

	Ref<IVGSSequenceComponent> IVGSSequenceComponentManager::CreateSequenceEntryByTypeNameID(
		const std::string& typeNameID)
	{
		auto it = RegisteredComponents.find(typeNameID);
		if (it != RegisteredComponents.end())
		{
			return it->second->Clone();
		}
		return nullptr;
	}

	Ref<IVGSSequenceComponent> CreateSequenceEntryByTypeNameID(const std::string& typeNameID)
	{
		return IVGSSequenceComponentManager::Get().CreateSequenceEntryByTypeNameID(typeNameID);
	}
}
