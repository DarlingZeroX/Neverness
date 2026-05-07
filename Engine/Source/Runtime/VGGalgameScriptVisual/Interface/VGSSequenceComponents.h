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
#include "../VGGalScriptVisualConfig.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "IVGSSequenceComponent.h"

namespace VisionGal
{
	template<class T>
	struct TVGSSequenceComponent:public IVGSSequenceComponent
	{
		~TVGSSequenceComponent() override = default;

		static std::string StaticGetTypeNameID()
		{
			return T{}.GetTypeNameID();
		}

		Ref<IVGSSequenceComponent> Clone() override
		{
			auto instance = MakeRef<T>();
			*instance = * dynamic_cast<T*>(this);
			return instance;
		}
	};

	struct VG_GALGAME_SCRIPT_VISUAL_API VGSSC_CommonDialogue : public TVGSSequenceComponent<VGSSC_CommonDialogue>
	{
		VGSSC_CommonDialogue();
		~VGSSC_CommonDialogue() override = default;

		std::string GetTypeNameID() override { return "CommonDialogue"; }

		VGSCharacterObjectID CharacterID;
		std::string DialogueText;
	};

	struct VG_GALGAME_SCRIPT_VISUAL_API VGSSC_ChangeFigure : public TVGSSequenceComponent<VGSSC_ChangeFigure>
	{
		VGSSC_ChangeFigure();
		~VGSSC_ChangeFigure() override = default;

		std::string GetTypeNameID() override { return "ChangeFigure"; }

		VGSCharacterObjectID CharacterID;
	};

	struct VG_GALGAME_SCRIPT_VISUAL_API VGSSC_ShowPicture : public TVGSSequenceComponent<VGSSC_ShowPicture>
	{
		VGSSC_ShowPicture();
		~VGSSC_ShowPicture() override = default;

		std::string GetTypeNameID() override { return "ShowPicture"; }
	};
}
