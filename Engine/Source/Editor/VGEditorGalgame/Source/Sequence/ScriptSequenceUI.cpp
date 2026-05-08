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

#include "Sequence/ScriptSequenceUI.h"
#include "Sequence/SequenceComponentTableUI.h"
#include <VGImgui/IncludeImGui.h>

#include "HFileSystem/Interface/HFileSystem.h"
#include "Sequence/ComponentDrawerRegistry.h"
#include "VGGalgameScriptVisual/Include/VisualSequence/SequenceDataContainerSerialization.h"

namespace VisionGal::Editor
{
	VGScriptSequenceUI::VGScriptSequenceUI()
	{
		m_SequenceData = MakeRef<VGSSequenceDataContainer>();
		m_SequenceData->AppendEntry(CreateSequenceEntryByTypeNameID(VGSSC_CommonDialogue::StaticGetTypeNameID()));
		m_SequenceData->AppendEntry(CreateSequenceEntryByTypeNameID(VGSSC_ChangeFigure::StaticGetTypeNameID()));
		m_SequenceData->AppendEntry(CreateSequenceEntryByTypeNameID(VGSSC_ChangeBackground::StaticGetTypeNameID()));
		m_SequenceData->AppendEntry(CreateSequenceEntryByTypeNameID(VGSSC_CommonDialogue::StaticGetTypeNameID()));

		m_EntryUI.OnIconClicked.Subscribe([this](const std::string& typeNameID) {
			// 这里可以根据 typeNameID 来创建对应的 SequenceEntry，并添加到 m_SequenceData 中
			auto entry = CreateSequenceEntryByTypeNameID(typeNameID);
			m_SequenceData->AppendEntry(entry);
			});

	}

	VGScriptSequenceUI::~VGScriptSequenceUI()
	{
	}

	void VGScriptSequenceUI::RenderSequenceUI()
	{
		ImGui::ShowDemoWindow();

		if (ImGui::Begin("Top Menu"))
		{
			m_EntryUI.ShowDemoIconsUI();
		}

		ImGui::End();

		unsigned int index = 0;
		std::vector<unsigned> removeIndices;
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

		int dragSourceIndex = -1;
		int dragTargetIndex = -1;

		for (auto& entry : m_SequenceData->m_Sequence)
		{
			std::string header = entry->GetTypeNameID() + std::to_string(entry->SequenceIndex);
			bool show = true;

			// Begin drag-drop target before header
			ImGui::PushID(index);

			ImGui::Button(std::to_string(index).c_str());
			ImGui::SameLine();
			bool showContent = ImGui::CollapsingHeader(header.c_str(), &show, flags);

			// Drag source
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				std::string payloadInfo = header;
				if (ImGui::IsKeyDown(ImGuiKey_Space))
					payloadInfo= header + " (Space to copy)";

				ImGui::SetDragDropPayload("SEQ_ENTRY_INDEX", &index, sizeof(int));
				ImGui::Selectable(payloadInfo.c_str());
				ImGui::EndDragDropSource();
			}

			// Drag target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SEQ_ENTRY_INDEX"))
				{
					int payloadIndex = *(const int*)payload->Data;
					if (payloadIndex != index)
					{
						dragSourceIndex = payloadIndex;
						dragTargetIndex = index;
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (showContent)
			{
				GalSeqComDrawerRegistry::GetInstance().GetDrawer(entry->GetTypeNameID())->OnGUI(index, entry.get());
			}

			if (!show)
				removeIndices.push_back(index);

			ImGui::PopID();
			index++;
		}

		// 处理拖拽排序
		if (dragSourceIndex != -1 && dragTargetIndex != -1 && dragSourceIndex != dragTargetIndex)
		{
			// 如果往后拖，目标索引会变
			if (dragSourceIndex > dragTargetIndex)
				m_SequenceData->InsertBefore(dragSourceIndex, dragTargetIndex);
			else
				m_SequenceData->InsertBehind(dragSourceIndex, dragTargetIndex);
		}

		m_SequenceData->RemoveEntries(removeIndices);
	}

	void VGScriptSequenceUI::SaveTest()
	{
		Horizon::HFileSystem::WriteTextToFile("E:/TestSequence.json", SerializeVGSSequenceDataContainerToString(*m_SequenceData, 4));
	}
}
