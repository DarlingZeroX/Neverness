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

#include "GalComDrawer.h"
#include <NNEditorFramework/Include/EditorCore/Localization.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeImGui/Include/ImGuiEx/ImGuiVector.h>
#include <VGGalgame/Include/GalGameEngine.h>
#include <VGGalgameCore/Include/Components.h>
#include "VGGalgameCore/Interface/IStoryScript.h"
#include <NNRuntimeAsset/Interface/Package.h>
#include <VGGalgameCore/Include/GalGameEngineAccess.h>

#include "NNRuntimeAsset/Include/GalGameAsset.h"
#include "VGGalgameSequenceRuntime/Include/Asset/Asset.h"

namespace VisionGal::Editor
{
	void GalGameEngineComponentDrawer::OnGUI(IEntity* entity)
	{
		if (ImGui::CollapsingHeader(EditorText{ "GalGame Engine" }.c_str(), ImGuiTreeNodeFlags_DefaultOpen) == false)
			return;

		auto* com = entity->GetComponent<GalGame::GalGameEngineComponent>();
		if (com == nullptr)
			return;

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

		if (ImGui::BeginTable("GalGameEngineComponentDrawerTable", 2, flags))
		{
			// 修复SetNextItemWidth逐渐变宽的问题
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// 剧情脚本设置
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Story Script" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
				ImGuiEx::InputText("##GalGameStoryScript", com->scriptPath, ImGuiInputTextFlags_ReadOnly);
				ScriptBeginDropTarget(com);
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES "##RemoveStoryScript"))
				{
					//com->script = nullptr;
					com->scriptPath = {};
				}
			}

			// 重新加载剧情脚本
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Reload Story Script" }.c_str());
				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button(ICON_FA_REDO "##Reload Story Script"))
				{
					GalGame::GalGameEngineAccess::Current()->GetSubsystemBus()->Script()->ReloadStoryScript();
				}
			}

			// 剧情选择
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Choice UI" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
				ImGuiEx::InputText("##GalGameChoiceUI", com->choiceUIPath, ImGuiInputTextFlags_ReadOnly);
				UIFileBeginDropTarget(com->choiceUIPath);
			}


			// 全屏文字
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Full screen text UI" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
				ImGuiEx::InputText("##Full screen text UI", com->fullScreenTextUIPath, ImGuiInputTextFlags_ReadOnly);
				UIFileBeginDropTarget(com->fullScreenTextUIPath);
			}


			// 输入
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(EditorText{ "Input UI" }.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
				ImGuiEx::InputText("##Input UI", com->inputUIPath, ImGuiInputTextFlags_ReadOnly);
				UIFileBeginDropTarget(com->inputUIPath);
			}

			ImGui::EndTable();
		}
	}

	const String GalGameEngineComponentDrawer::GetBindType() const
	{
		return GalGame::GalGameEngineComponent{}.GetComponentType();
	}

	void GalGameEngineComponentDrawer::ScriptBeginDropTarget(GalGame::GalGameEngineComponent* com)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string path = static_cast<char*>(payload->Data);

				// 检查文件类型
				auto assetType = VGPackage::GetAssetType(path);
				if (!GalGame::GalGameScriptExecutorFactory::Get().HasExecutor(assetType))
				{
					ImGuiEx::PushNotification({ ImGuiExToastType::Error, "错误文件类型!" });
					ImGui::EndDragDropTarget();
					return;
				}

				// 设置剧情脚本
				//com->script = GalGame::LuaStoryScript::LoadFromFile(path);
				com->scriptPath = path;

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "设置剧情脚本成功!" });
			}
			ImGui::EndDragDropTarget();
		}
	}

	void GalGameEngineComponentDrawer::UIFileBeginDropTarget(std::string& uiPath)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string path = static_cast<char*>(payload->Data);

				// 检查文件类型
				auto assetType = VGPackage::GetAssetType(path);
				if (assetType != "HTML")
				{
					ImGuiEx::PushNotification({ ImGuiExToastType::Error, "错误文件类型!" });
					ImGui::EndDragDropTarget();
					return;
				}

				uiPath = path;

				ImGuiEx::PushNotification({ ImGuiExToastType::Info, "设置UI文件成功!" });
			}
			ImGui::EndDragDropTarget();
		}
	}
}
