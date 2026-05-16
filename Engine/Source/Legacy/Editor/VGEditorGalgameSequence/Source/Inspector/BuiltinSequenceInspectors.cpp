/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/BuiltinSequenceInspectors.h"

#include "Inspector/SequenceInspectorRenderer.h"

#include "Commands/EditSequencePropertyCommand.h"
#include "Commands/SetSequenceEntryBoolPropertyCommand.h"
#include "Core/SequenceEditorContext.h"

#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeCore/Interface/Loader.h>

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		class SchemaBackedSequenceInspector final : public ISequenceInspector
		{
		public:
			explicit SchemaBackedSequenceInspector(SequenceComponentMetadata meta)
				: m_meta(std::move(meta))
			{
			}

			void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) override
			{
				if (component == nullptr)
					return;
				if (context != nullptr && context->document != nullptr && context->undo != nullptr)
					(void)SequenceInspectorRenderer::DrawFromSchema(m_meta, index, static_cast<void*>(component), context);
				else
					ImGui::TextUnformatted(u8"（无可写上下文，以下为只读预览：Schema 路径需撤销栈）");
			}

			std::string GetDisplayName() const override { return m_meta.PrimaryLabel(); }

			std::string GetBoundTypeNameID() const override { return m_meta.TypeNameID; }

		private:
			SequenceComponentMetadata m_meta;
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
				auto* com = dynamic_cast<VisionGal::VGSSC_ChangeBackground*>(component);
				if (com == nullptr)
					return;

				if (context == nullptr || context->document == nullptr || context->undo == nullptr)
				{
					ImGui::TextUnformatted(u8"（无可写上下文，属性只读）");
					return;
				}

				if (m_bgStagingIndex != index)
				{
					m_bgStagingIndex = index;
					m_bgStagingPath = com->TextureResourcePath;
				}

				if (com->Temp.PreviewTexture == nullptr && com->TextureResourcePath.empty() == false)
					if (auto tex = LoadObject<Texture2D>(com->TextureResourcePath))
						com->Temp.PreviewTexture = tex;

				if (com->ShowState)
				{
					ImTextureRef previewTexture;
					if (com->Temp.PreviewTexture != nullptr)
						previewTexture = com->Temp.PreviewTexture->GetTexture()->GetShaderResourceView();

					ImGui::ImageButton("Background", previewTexture, ImVec2(100, 100));
					TextureBeginDropTarget(com, index, context);
					ImGui::SameLine();
				}

				ImGuiEx::InputText(u8"纹理路径##BgTexPath", m_bgStagingPath);
				if (ImGui::IsItemDeactivatedAfterEdit())
					context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
						index, SequenceEditFieldId::ChangeBackground_TextureResourcePath, m_bgStagingPath));

				bool show = com->ShowState;
				if (ImGui::Checkbox(u8"显示背景##BgShow", &show))
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						index, SequenceEditBoolFieldId::ChangeBackground_ShowState, show));
				ImGui::SameLine();
				bool wait = com->Wait;
				if (ImGui::Checkbox(u8"等待##BgWait", &wait))
					context->ExecuteCommand(std::make_unique<SetSequenceEntryBoolPropertyCommand>(
						index, SequenceEditBoolFieldId::ChangeBackground_Wait, wait));
			}

			std::string GetDisplayName() const override { return m_displayName; }

			std::string GetBoundTypeNameID() const override
			{
				return VisionGal::VGSSC_ChangeBackground::StaticGetTypeNameID();
			}

		private:
			static void TextureBeginDropTarget(
				VisionGal::VGSSC_ChangeBackground* com,
				unsigned index,
				SequenceEditorContext* context)
			{
				if (ImGui::BeginDragDropTarget())
				{
					if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
					{
						std::string path = static_cast<char*>(payload->Data);

						if (LoadObject<Texture2D>(path))
						{
							context->ExecuteCommand(std::make_unique<EditSequencePropertyCommand>(
								index, SequenceEditFieldId::ChangeBackground_TextureResourcePath, path));
							com->Temp.PreviewTexture = nullptr;
							ImGuiEx::PushNotification({ ImGuiExToastType::Info, u8"已设置背景纹理路径" });
						}
						else
							ImGuiEx::PushNotification({ ImGuiExToastType::Warning, u8"设置背景纹理失败" });
					}
					ImGui::EndDragDropTarget();
				}
			}

			std::string m_displayName;
			unsigned m_bgStagingIndex = 0xFFFFFFFFu;
			std::string m_bgStagingPath;
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
			return std::make_unique<SchemaBackedSequenceInspector>(meta);
		if (id == VisionGal::VGSSC_ChangeFigure::StaticGetTypeNameID())
			return std::make_unique<SchemaBackedSequenceInspector>(meta);
		if (id == VisionGal::VGSSC_ChangeBackground::StaticGetTypeNameID())
			return std::make_unique<ChangeBackgroundSequenceInspector>(label);
		return std::make_unique<FallbackSequenceInspector>(id, label);
	}
}
