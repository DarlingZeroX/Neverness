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
#include <VGGalgameScriptSequence/Include/Sequence/Components.h>
#include <VGImgui/Include/Imgui/imgui.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGCore/Interface/Loader.h>

namespace VisionGal::Editor
{
	void GSCD_CommonDialogue::OnGUI(unsigned int index,IVGSSequenceComponent* entry)
	{
		VGSSC_CommonDialogue* com = dynamic_cast<VGSSC_CommonDialogue*>(entry);
		if (com == nullptr)
			return;

		ImGuiEx::InputText(
			"##DialogueCharacterName", 
			com->DialogueCharacterName
		);
		ImGuiEx::InputTextMultiline(
			"##DialogueText", 
			com->DialogueText, 
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

		if (com->ShowState)
		{
			ImGui::ImageButton("Background", ImTextureRef(), ImVec2(100, 100));
			ImGui::SameLine();
		}
		ImGui::Checkbox("Show Figure", &com->ShowState);
		ImGui::SameLine();
		ImGui::Checkbox("Wait", &com->Wait);
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

		if (com->Temp.PreviewTexture == nullptr && com->TextureResourcePath.empty() == false)
		if (auto tex = LoadObject<Texture2D>(com->TextureResourcePath))
		{
			com->Temp.PreviewTexture = tex;
		}
		
		if (com->ShowState)
		{
			ImTextureRef previewTexture;
			if (com->Temp.PreviewTexture != nullptr){
				previewTexture = com->Temp.PreviewTexture->GetTexture()->GetShaderResourceView();
			}

			ImGui::ImageButton("Background", previewTexture, ImVec2(100, 100));
			TextureBeginDropTarget(com);
			ImGui::SameLine();
		}
		ImGui::Checkbox("Show Figure", &com->ShowState);
		ImGui::SameLine();
		ImGui::Checkbox("Wait", &com->Wait);
	}

	const std::string GSCD_ChangeBackground::GetBindType() const
	{
		return VGSSC_ChangeBackground::StaticGetTypeNameID();
	}

	void GSCD_ChangeBackground::TextureBeginDropTarget(IVGSSequenceComponent* entry)
	{
		VGSSC_ChangeBackground* com = dynamic_cast<VGSSC_ChangeBackground*>(entry);

		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string path = static_cast<char*>(payload->Data);

				if (auto tex = LoadObject<Texture2D>(path))
				{
					com->Temp.PreviewTexture = tex;
					com->TextureResourcePath = path;
					ImGuiEx::PushNotification({ ImGuiExToastType::Info, "设置视频成功!" });
				}
				else
				{
					ImGuiEx::PushNotification({ ImGuiExToastType::Warning, "设置视频失败!" });
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}
