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

#include "Sequence/ComponentDrawers.h"
#include <VGGalgameScriptVisual/Include/VisualSequence/SequenceComponents.h>
#include <VGImgui/Include/Imgui/imgui.h>
#include <VGImgui/IncludeImGuiEx.h>

namespace VisionGal::Editor
{
	void GSCD_CommonDialogue::OnGUI(unsigned int index,IVGSSequenceComponent* entry)
	{
		VGSSC_CommonDialogue* com = dynamic_cast<VGSSC_CommonDialogue*>(entry);
		if (com == nullptr)
			return;

		ImGuiEx::InputText(
			"##DialogueCharacterName", 
			com->dialogueCharacterName
		);
		ImGuiEx::InputTextMultiline(
			"##DialogueText", 
			com->dialogueText, 
			ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4)
		);
	}

	const std::string GSCD_CommonDialogue::GetBindType() const
	{
		return VGSSC_CommonDialogue::StaticGetTypeNameID();
	}

	void GSCD_ChangeFigure::OnGUI(unsigned int index,IVGSSequenceComponent* entry)
	{
		VGSSC_ChangeFigure* com = dynamic_cast<VGSSC_ChangeFigure*>(entry);
		if (com == nullptr)
			return;

		if (com->showState)
		{
			ImGui::ImageButton("Background", ImTextureRef(), ImVec2(100, 100));
			ImGui::SameLine();
		}
		ImGui::Checkbox("Show Figure", &com->showState);
		ImGui::SameLine();
		ImGui::Checkbox("Wait", &com->wait);
	}

	const std::string GSCD_ChangeFigure::GetBindType() const
	{
		return VGSSC_ChangeFigure::StaticGetTypeNameID();
	}

	void GSCD_ChangeBackground::OnGUI(unsigned int index, IVGSSequenceComponent* entry)
	{
		VGSSC_ChangeBackground* com = dynamic_cast<VGSSC_ChangeBackground*>(entry);
		if (com == nullptr)
			return;
		
		if (com->showState)
		{
			ImGui::ImageButton("Background", ImTextureRef(), ImVec2(100, 100));
			ImGui::SameLine();
		}
		ImGui::Checkbox("Show Figure", &com->showState);
		ImGui::SameLine();
		ImGui::Checkbox("Wait", &com->wait);
	}

	const std::string GSCD_ChangeBackground::GetBindType() const
	{
		return VGSSC_ChangeBackground::StaticGetTypeNameID();
	}
}
