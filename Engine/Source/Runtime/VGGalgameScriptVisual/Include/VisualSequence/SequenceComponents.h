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
#include "../../VGGalScriptVisualConfig.h"
#include "../../Interface/IVGSSequenceComponent.h"
#include "VGGalgameCore/Interface/IGameObject.h"

namespace VisionGal
{
	
	struct VG_GALGAME_VISUAL_SCRIPT_API VGSSC_CommonDialogue : public TVGSSequenceComponent<VGSSC_CommonDialogue>
	{
		VGSSC_CommonDialogue();
		~VGSSC_CommonDialogue() override = default;

		std::string GetTypeNameID() override { return "CommonDialogue"; }

		VGSSCharacterObjectID characterID = 0;
		std::string dialogueCharacterName;
		std::string dialogueText;

		bool wait = true;
	};

	struct VG_GALGAME_VISUAL_SCRIPT_API VGSSC_ChangeFigure : public TVGSSequenceComponent<VGSSC_ChangeFigure>
	{
		VGSSC_ChangeFigure();
		~VGSSC_ChangeFigure() override = default;

		std::string GetTypeNameID() override { return "ChangeFigure"; }

		VGSSCharacterObjectID characterID = 0;
		VGSSSpriteObjectID textureID = 0;
		std::string textureResourcePath;
		bool showState = true;
		bool wait = true;
	};

	struct VG_GALGAME_VISUAL_SCRIPT_API VGSSC_ChangeBackground : public TVGSSequenceComponent<VGSSC_ChangeBackground>
	{
		VGSSC_ChangeBackground() = default;
		~VGSSC_ChangeBackground() override = default;

		std::string GetTypeNameID() override { return "ChangeBackground"; }

		VGSSSpriteObjectID textureID = 0;
		std::string textureResourcePath;
		bool showState = true;
		bool wait = true;
	};

	//struct VG_GALGAME_SCRIPT_VISUAL_API VGSSC_ShowPicture : public TVGSSequenceComponent<VGSSC_ShowPicture>
	//{
	//	VGSSC_ShowPicture() = default;
	//	~VGSSC_ShowPicture() override = default;
	//
	//	std::string GetTypeNameID() override { return "ShowPicture"; }
	//
	//	VGSSTextureObjectID textureID = 0;
	//	bool showState = true;
	//	bool wait = true;
	//};
}
