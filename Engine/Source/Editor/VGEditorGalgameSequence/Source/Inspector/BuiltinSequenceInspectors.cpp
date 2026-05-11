/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/BuiltinSequenceInspectors.h"

#include "Commands/EditSequencePropertyCommand.h"
#include "Core/SequenceEditorContext.h"

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGCore/Interface/Loader.h>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		class CommonDialogueSequenceInspector final : public ISequenceInspector
		{
		public:
			explicit CommonDialogueSequenceInspector(std::string displayName)
				: m_displayName(std::move(displayName))
			{
			}

			void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override
			{
				if (component == nullptr)
					return;

				auto* com = dynamic_cast<VisionGal::VGSSC_CommonDialogue*>(component);
				if (com == nullptr)
					return;

				if (context != nullptr && context->document != nullptr && context->undo != nullptr)
				{
					if (m_commonStagingIndex != index)
					{
						m_commonStagingIndex = index;
						m_commonStagingName = com->DialogueCharacterName;
						m_commonStagingText = com->DialogueText;
					}

					ImGuiEx::InputText("##DialogueCharacterName", m_commonStagingName);
					if (ImGui::IsItemDeactivatedAfterEdit())
						context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
							index, SequenceEditFieldId::CommonDialogue_CharacterName, m_commonStagingName));

					ImGuiEx::InputTextMultiline(
						"##DialogueText",
						m_commonStagingText,
						ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
					if (ImGui::IsItemDeactivatedAfterEdit())
						context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
							index, SequenceEditFieldId::CommonDialogue_DialogueText, m_commonStagingText));
					return;
				}

				ImGuiEx::InputText("##DialogueCharacterName", com->DialogueCharacterName);
				ImGuiEx::InputTextMultiline(
					"##DialogueText",
					com->DialogueText,
					ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
			}

			std::string GetDisplayName() const override { return m_displayName; }

			std::string GetBoundTypeNameID() const override
			{
				return VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID();
			}

		private:
			std::string m_displayName;
			unsigned m_commonStagingIndex = 0xFFFFFFFFu;
			std::string m_commonStagingName;
			std::string m_commonStagingText;
		};

		class ChangeFigureSequenceInspector final : public ISequenceInspector
		{
		public:
			explicit ChangeFigureSequenceInspector(std::string displayName)
				: m_displayName(std::move(displayName))
			{
			}

			void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override
			{
				(void)index;
				(void)context;
				auto* com = dynamic_cast<VisionGal::VGSSC_ChangeFigure*>(component);
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

			std::string GetDisplayName() const override { return m_displayName; }

			std::string GetBoundTypeNameID() const override
			{
				return VisionGal::VGSSC_ChangeFigure::StaticGetTypeNameID();
			}

		private:
			std::string m_displayName;
		};

		class ChangeBackgroundSequenceInspector final : public ISequenceInspector
		{
		public:
			explicit ChangeBackgroundSequenceInspector(std::string displayName)
				: m_displayName(std::move(displayName))
			{
			}

			void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override
			{
				(void)index;
				(void)context;
				auto* com = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(component);
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
					if (com->Temp.PreviewTexture != nullptr)
						previewTexture = com->Temp.PreviewTexture->GetTexture()->GetShaderResourceView();

					ImGui::ImageButton("Background", previewTexture, ImVec2(100, 100));
					TextureBeginDropTarget(com);
					ImGui::SameLine();
				}
				ImGui::Checkbox("Show Figure", &com->ShowState);
				ImGui::SameLine();
				ImGui::Checkbox("Wait", &com->Wait);
			}

			std::string GetDisplayName() const override { return m_displayName; }

			std::string GetBoundTypeNameID() const override
			{
				return VisionGal::VGSSC_ChangeBackground::StaticGetTypeNameID();
			}

		private:
			static void TextureBeginDropTarget(VisionGal::VGSSC_ChangeBackground* com)
			{
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
							ImGuiEx::PushNotification({ ImGuiExToastType::Warning, "设置视频失败!" });
					}
					ImGui::EndDragDropTarget();
				}
			}

			std::string m_displayName;
		};

		class FallbackSequenceInspector final : public ISequenceInspector
		{
		public:
			FallbackSequenceInspector(std::string typeNameID, std::string displayName)
				: m_typeNameID(std::move(typeNameID))
				, m_displayName(std::move(displayName))
			{
			}

			void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override
			{
				(void)index;
				(void)component;
				(void)context;
			}

			std::string GetDisplayName() const override { return m_displayName; }

			std::string GetBoundTypeNameID() const override { return m_typeNameID; }

		private:
			std::string m_typeNameID;
			std::string m_displayName;
		};
	}

	std::unique_ptr<ISequenceInspector> MakeSequenceInspectorForMetadata(const SequenceComponentMetadata& meta)
	{
		const std::string& id = meta.TypeNameID;
		const std::string label = meta.PrimaryLabel();

		if (id == VisionGal::VGSSC_CommonDialogue::StaticGetTypeNameID())
			return std::make_unique<CommonDialogueSequenceInspector>(label);
		if (id == VisionGal::VGSSC_ChangeFigure::StaticGetTypeNameID())
			return std::make_unique<ChangeFigureSequenceInspector>(label);
		if (id == VisionGal::VGSSC_ChangeBackground::StaticGetTypeNameID())
			return std::make_unique<ChangeBackgroundSequenceInspector>(label);
		return std::make_unique<FallbackSequenceInspector>(id, label);
	}
}
