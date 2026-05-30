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

#include "SceneBrowser.h"
#include "NNEditorFrameworkLegacy/Include/EditorCore/Localization.h"
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNEngineLegacy/Include/Scene/SceneLegacy.h>
#include <NNRuntimeCore/Include/Core/EventBus.h>
#include <NNEngineLegacy/Include/Scene/Components.h>
#include <NNEngineLegacy/Include/Scene/GameActorFactory.h>

namespace NN::Editor
{
	SceneBrowserPanel::SceneBrowserPanel()
		:m_rScene(nullptr)
	{
		SubscribeEngineEvent();

		//for (auto& actorType: GetGameActorFactory()->GetActorTypeList())
		//{
		//	m_Menu.AddMenuItem({ actorType, "", nullptr, true, [this] { CreateGameActor("Camera"); } });
		//}
		m_Menu.AddMenuItem({EditorText{"Camera"}.c_str(), "", nullptr, true, [this] { CreateGameActor("Camera"); } });
		m_Menu.AddMenuItem({EditorText{"Sprite"}.c_str(), "", nullptr, true, [this] { CreateGameActor("Sprite"); } });
		m_Menu.AddMenuItem({EditorText{"Audio Source"}.c_str(), "", nullptr, true, [this] { CreateGameActor("AudioSource"); } });
		m_Menu.AddMenuItem({EditorText{"Video Player"}.c_str(), "", nullptr, true, [this] { CreateGameActor("VideoPlayer"); } });
		m_Menu.AddMenuItem({EditorText{"UI Document"}.c_str(), "", nullptr, true, [this] { CreateGameActor("UIDocument"); } });
		m_Menu.AddMenuItem({EditorText{"GalGame Engine"}.c_str(), "", nullptr, true, [this] { CreateGameActor("GalGameEngine"); } });
	}

	void SceneBrowserPanel::SubscribeEngineEvent()
	{
		Runtime::EngineEventBus::Get().OnEngineEvent.Subscribe([this](const Runtime::EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case Runtime::EngineEventType::MainSceneChanged:
					m_rScene = static_cast<Runtime::SceneLegacy*>(evt.Scene);
					break;
				}
			});
	}

	void SceneBrowserPanel::OnGUI()
	{
		if (!m_IsOpen)
			return;

		ImVec4* colors = ImGui::GetStyle().Colors;
		ImGuiEx::ScopedStyleColor winClr(ImGuiCol_WindowBg, ImGui::ColorConvertFloat4ToU32(colors[ImGuiCol_MenuBarBg]));

		//static EditorTitle title(ICON_FA_SITEMAP, "World Hierarchy");
		//static NN::Core::HText labelText("Label");
		//static NN::Core::HText typeText("Type");

		if (ImGui::Begin(GetWindowFullName().c_str()))
		{
			//ImGuiEx::ScopedWindow window(GetWindowFullName().c_str());

			if (m_rScene == nullptr)
			{
				ImGui::End();
				return;
			}

			m_IsAnyItemHovered = false;

			static std::string buf;
			ImGuiEx::InputTextWithHint("##World Hierarchy Search", "Search...", buf);

			//static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoBordersInBody;

			auto& rootRelation = *m_rScene->GetSceneActor()->GetComponent<NN::Core::HRelationship>();
			DrawHierarchy(*m_rScene->GetSceneActor(), rootRelation);

			//bottom overlay
			TickBottomOverlayUI();
			PanelContextMenu();
		}

		ImGui::End();
	}

	std::string SceneBrowserPanel::GetWindowFullName()
	{
		return EditorText{ "Scene Browser",ICON_FA_LIST_UL }.GetText();
	}

	std::string SceneBrowserPanel::GetWindowName()
	{
		return "Scene Browser";
	}

	void SceneBrowserPanel::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool SceneBrowserPanel::IsWindowOpened()
	{
		return m_IsOpen;
	}

	void SceneBrowserPanel::PanelContextMenu()
	{
		if (!m_IsAnyItemHovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("SceneBrowserPanelContextMenu");
		}

		if (ImGui::BeginPopup("SceneBrowserPanelContextMenu")) // <-- use last item id as popup id
		{
			m_Menu.OnGUI();

			ImGui::EndPopup();
		}
	}

	void SceneBrowserPanel::CreateGameActor(const Runtime::String& type, Runtime::IEntity* parent) const
	{
		Runtime::GetGameActorFactory()->CreateActor(m_rScene, type, parent);
	}

	void SceneBrowserPanel::TickBottomOverlayUI()
	{
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 overlayPosBegin = ImVec2(windowPos.x, windowPos.y + windowSize.y - 20);
		ImVec2 overlayPosEnd = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

		size_t enttSize = m_rScene->GetWorld()->view<Runtime::TransformComponent>().size();
		//size_t enttSize = m_rScene.GetWorld().count<Scene::TransformComponent>();
		//size_t enttSize = m_rScene.GetWorld().count(Scene::TransformComponent::GetComponentId());
		std::string overlayText = std::to_string(enttSize) + " actors";

		ImGui::PushClipRect(overlayPosBegin, overlayPosEnd, true);
		ImGui::GetWindowDrawList()->AddText(ImVec2(overlayPosBegin.x + 10.0f, overlayPosBegin.y), IM_COL32_WHITE, overlayText.c_str());
		ImGui::PopClipRect();
	}

	void SceneBrowserPanel::DrawHierarchy(Runtime::IGameActor& parent, NN::Core::HRelationship& parentRelation)
	{
		//ImVec2 size = ImGui::GetWindowSize();
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 windowEndPos = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

		const ImVec2 p0 = ImGui::GetItemRectMin();

		//Invisible clip
		if (p0.y > windowEndPos.y)
		{
			ImGui::TableNextRow();
			return;
		}

		if (!DrawTreeNodeChildRow(&parent))
			return;

		Runtime::IGameActor* currentChild = dynamic_cast<Runtime::IGameActor*>( parentRelation.FirstChild );

		for (int child_n = 0; child_n < parentRelation.ChildrenCount; child_n++)
		{
			NN::Core::HRelationship& chlidRelation = *currentChild->GetComponent<NN::Core::HRelationship>();
			auto* nextSibling = chlidRelation.NextSibling;

			if (chlidRelation.ChildrenCount != 0)
			{
				DrawHierarchy(*currentChild, chlidRelation);
			}
			else
			{
				//Invisible clip
				const ImVec2 p1 = ImGui::GetCursorScreenPos();	//float p1y = p1.y + ImGui::GetFrameHeight();
				if (p1.y > windowEndPos.y)
				{
				}

				DrawTreeNodeChildRow(currentChild);
			}

			currentChild = dynamic_cast<Runtime::IGameActor*>(nextSibling);
		}
		ImGui::TreePop();
	}

	bool SceneBrowserPanel::DrawTreeNodeRow(Runtime::IGameActor& parent)
	{
		const NN::Core::HRelationship& parentLabel = *parent.GetComponent<NN::Core::HRelationship>();

		ImGuiEx::ScopedID id(&parent);

		ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_SpanFullWidth ;

		// 如果选中了
		if (parent.GetEntityID() == m_SelectedEntityID)
			parentFlags |= ImGuiTreeNodeFlags_Selected;

		// 如果是场景
		if (parent.GetEntityID() == 0)
			parentFlags |= ImGuiTreeNodeFlags_DefaultOpen;

		bool open = ImGui::TreeNodeEx(parentLabel.Label.c_str(), parentFlags);
		HandleItemHovered(parent);

		return open;
	}

	bool SceneBrowserPanel::DrawTreeNodeChildRow(Runtime::IGameActor* actor)
	{
		const NN::Core::HRelationship& relationship = *actor->GetComponent<NN::Core::HRelationship>();

		ImGuiEx::ScopedID id(actor->GetEntityID());

		ImGuiTreeNodeFlags flags;
		if (relationship.ChildrenCount > 0 || actor->GetEntityID() == 0)
			flags = ImGuiTreeNodeFlags_SpanFullWidth;
		else
			flags = ImGuiTreeNodeFlags_Leaf |ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

		// 如果选中了
		if (actor->GetEntityID() == m_SelectedEntityID)
			flags |= ImGuiTreeNodeFlags_Selected;

		// 如果是场景
		if (actor->GetEntityID() == 0)
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		std::string label = ICON_FA_DICE_D6 " ";
		label += relationship.Label;

		ImGui::TreeNodeEx(label.c_str(), flags);
		HandleItemHovered(*actor);

		return true;
	}

	void SceneBrowserPanel::HandleItemHovered(Runtime::IGameActor& entityID)
	{
		ItemContextMenu(entityID);

		if (!ImGui::IsItemHovered())
			return;

		m_IsAnyItemHovered = true;

		ImGui::BeginTooltip();
		Runtime::String tip = "ActorID: " + std::to_string(entityID.GetEntityID());
		ImGui::Text(tip.c_str());
		ImGui::EndTooltip();
		
		if (ImGui::IsItemHovered(ImGuiMouseButton_Left) && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			TriggerSelectedEvent(entityID);

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("PLACE_GAME_ACTOR", &entityID, sizeof(entityID));
			ImGui::Text(tip.c_str());

			ImGui::EndDragDropSource();
		}
	}

	bool SceneBrowserPanel::ItemContextMenu(Runtime::IGameActor& entityID)
	{
		if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
		{
			if (ImGui::MenuItem(EditorText{ "Remove" }.c_str()))
			{
				m_rScene->RemoveActor(entityID.GetEntityID());

				ImGui::EndPopup();
				return true;
			}
			ImGui::Separator();

			ImGui::EndPopup();
		}

		return false;
	}

	void SceneBrowserPanel::TriggerSelectedEvent(Runtime::IGameActor& entity)
	{
		const auto* entityInfo = entity.GetComponent<NN::Core::HEntityObjectData>();

		Runtime::SceneEvent evt;
		evt.EventType = Runtime::SceneEventType::ActorSelected;
		evt.ActorID = entityInfo->ID;
		Runtime::EngineEventBus::Get().OnSceneEvent.Invoke(evt);

		m_SelectedEntityID = entity.GetEntityID();
	}
}
