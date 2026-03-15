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

#include "EditorComponents/DetailBrowser/Browser.h"
#include "EditorComponents/DetailBrowser/ComponentDrawer.h"
#include "EditorCore/Localization.h"
#include <VGCore/Include/Core/EventBus.h>
#include <VGImgui/IncludeImGui.h>
#include <HCore/Interface/HStringTools.h>

#include "VGEngine/Include/Asset/Package.h"
#include "VGEngine/Include/Lua/LuaScript.h"
#include "VGImgui/Include/ImGuiEx/ImGuiEx.h"
#include "VGImgui/Include/ImGuiEx/ImNotify.h"

namespace VisionGal::Editor {

	DetailBrowserPanel::DetailBrowserPanel()
	{
		m_DrawerManager.RegisterDrawer(CreateRef<TransformComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<CameraComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<SpriteRendererComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<GalGameEngineComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<ScriptComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<AudioSourceComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<VideoPlayerComponentDrawer>());
		m_DrawerManager.RegisterDrawer(CreateRef<RmlUIDocumentComponentDrawer>());

		SubscribeEngineEvent();
		SubscribeSceneEvent();
	}

	void DetailBrowserPanel::OnGUI()
	{
		if (!m_IsOpen)
			return;

		if (m_pScene == nullptr)
			return;

		if (ImGui::Begin(GetWindowFullName().c_str()))
		{
			auto* selectedEntity = m_pScene->GetActor(m_SelectEntityID);
			if (m_SelectEntityID == 0 || selectedEntity == nullptr)
			{
				ImGui::Separator();
				ImGui::SameLine(5);
				ImGui::Text(EditorText{ "Select a game object to view details." }.c_str());

				ImGui::End();
				return;
			}

			//auto& registry = m_rScene.GetWorld();
			auto* entityObject = selectedEntity->GetComponent<Horizon::HEntityObjectData>();
			auto* relation = selectedEntity->GetComponent<Horizon::HRelationship>();

			if (entityObject != nullptr && entityObject->ComponentTypes.size() > 0)
			{
				//ImGui::TextWrapped(relation->Label.c_str());
				ImGui::Text(EditorText{ "Label" }.c_str());
				ImGui::SameLine();
				ImGuiEx::InputText("##DetailBrowserPanelActorLabel", relation->Label);

				for (auto& component : entityObject->ComponentTypes)
				{
					m_DrawerManager.GetDrawer(component)->OnGUI(selectedEntity);
				}
			}

			// 1. 获取窗口尺寸
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();

			// 2. 创建覆盖整个窗口的透明按钮（无视觉效果，仅用于交互）
			ImGui::SetCursorScreenPos(windowPos);
			ImGui::InvisibleButton("##DetailBrowserPanelDragTarget", windowSize);
			BeginDropTarget();
		}

		ImGui::End();
	}

	void DetailBrowserPanel::SubscribeEngineEvent()
	{
		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::MainSceneChanged:
					m_pScene = dynamic_cast<Scene*>(evt.Scene);
					break;
				}
			});
	}

	void DetailBrowserPanel::SubscribeSceneEvent()
	{
		EngineEventBus::Get().OnSceneEvent.Subscribe([this](const SceneEvent& evt)
			{
				switch (evt.EventType)
				{
				case SceneEventType::ActorSelected:
					m_SelectEntityID = evt.ActorID;
					break;
				case SceneEventType::ActorRemoved:
					break;
				}
			});
	}

	void DetailBrowserPanel::BeginDropTarget()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const auto* payload = ImGui::AcceptDragDropPayload("PLACE_CONTENT_BROWSER_ITEM"))
			{
				std::string path = static_cast<char*>(payload->Data);

				// 检查文件类型
				auto assetType = VGPackage::GetAssetType(path);
				if (assetType == "LuaScript")
				{
					if (auto* selectedEntity = dynamic_cast<GameActor*>( m_pScene->GetActor(m_SelectEntityID) ))
					{
						if (selectedEntity->GetComponent<ScriptComponent>() == nullptr)
						{
							selectedEntity->AddComponent<ScriptComponent>();
						}

						auto* com = selectedEntity->GetComponent<ScriptComponent>();
						auto script = LuaScript::LoadFromFile(path);
						com->scripts.push_back(script);

						ImGuiEx::PushNotification({ ImGuiExToastType::Info, "设置脚本成功!" });
					}
				}
				else
				{
					ImGuiEx::PushNotification({ ImGuiExToastType::Warning, "设置脚本失败!" });
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	std::string DetailBrowserPanel::GetWindowFullName()
	{
		return EditorText{ GetWindowName() }.GetText();
	}

	std::string DetailBrowserPanel::GetWindowName()
	{
		return "Details";
	}

	void DetailBrowserPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool DetailBrowserPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}
}
