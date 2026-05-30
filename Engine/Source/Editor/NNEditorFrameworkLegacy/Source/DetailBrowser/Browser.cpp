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

#include "DetailBrowser/Browser.h"
#include "DetailBrowser/ComponentDrawer.h"
#include "NNEditorFrameworkLegacy/Include/EditorCore/Localization.h"
#include <NNRuntimeCore/Include/Core/EventBus.h>
#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNCore/Interface/HStringTools.h>

#include "NNRuntimeAssetLegacy/Interface/Package.h"
#include "NNEngineLegacy/Include/Lua/LuaScript.h"
#include "NNRuntimeImGui/Include/ImGuiEx/ImGuiEx.h"
#include "NNRuntimeImGui/Include/ImGuiEx/ImNotify.h"

namespace NN::Editor {

	DetailBrowserPanel::DetailBrowserPanel()
	{
		auto& drawerRegistry = ComponentDrawerRegistry::GetInstance();
		drawerRegistry.RegisterDrawer(MakeRef<TransformComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<CameraComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<SpriteRendererComponentDrawer>());
		//drawerRegistry.RegisterDrawer(MakeRef<GalGameEngineComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<ScriptComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<AudioSourceComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<VideoPlayerComponentDrawer>());
		drawerRegistry.RegisterDrawer(MakeRef<RmlUIDocumentComponentDrawer>());

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
			auto* entityObject = selectedEntity->GetComponent<NN::Core::HEntityObjectData>();
			auto* relation = selectedEntity->GetComponent<NN::Core::HRelationship>();

			if (entityObject != nullptr && entityObject->ComponentTypes.size() > 0)
			{
				//ImGui::TextWrapped(relation->Label.c_str());
				ImGui::Text(EditorText{ "Label" }.c_str());
				ImGui::SameLine();
				ImGuiEx::InputText("##DetailBrowserPanelActorLabel", relation->Label);

				for (auto& component : entityObject->ComponentTypes)
				{
					auto& drawerRegistry = ComponentDrawerRegistry::GetInstance();
					drawerRegistry.GetDrawer(component)->OnGUI(selectedEntity);
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
		Runtime::EngineEventBus::Get().OnEngineEvent.Subscribe([this](const Runtime::EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case Runtime::EngineEventType::MainSceneChanged:
					m_pScene = dynamic_cast<Runtime::SceneLegacy*>(evt.Scene);
					break;
				}
			});
	}

	void DetailBrowserPanel::SubscribeSceneEvent()
	{
		Runtime::EngineEventBus::Get().OnSceneEvent.Subscribe([this](const Runtime::SceneEvent& evt)
			{
				switch (evt.EventType)
				{
				case Runtime::SceneEventType::ActorSelected:
					m_SelectEntityID = evt.ActorID;
					break;
				case Runtime::SceneEventType::ActorRemoved:
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
				auto assetType = Runtime::VGPackage::GetAssetType(path);
				if (assetType == "LuaScript")
				{
					if (auto* selectedEntity = dynamic_cast<Runtime::IGameActor*>( m_pScene->GetActor(m_SelectEntityID) ))
					{
						if (selectedEntity->GetComponent<Runtime::ScriptComponent>() == nullptr)
						{
							selectedEntity->AddComponent<Runtime::ScriptComponent>();
						}

						auto* com = selectedEntity->GetComponent<Runtime::ScriptComponent>();
						auto script = Runtime::LuaScript::LoadFromFile(path);
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
