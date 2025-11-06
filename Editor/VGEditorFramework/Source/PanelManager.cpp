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

#include "pch.h"
#include "PanelManager.h"

namespace VisionGal::Editor {

	PanelManager::PanelManager()
	{
		// Submit top-level dockspace
		m_TopLevelClass.ClassId = ImHashStr("TopLevelClass");
		m_TopLevelClass.DockingAllowUnclassed = false;
	}

	void PanelManager::AddPanel(const Ref<IEditorPanel>& panel)
	{
		m_Panels.push_back(panel);
	}

	void PanelManager::AddPanelWithID(const std::string& id, const Ref<IEditorPanel>& panel)
	{
		m_IDPanels[id] = panel;
	}

	IEditorPanel* PanelManager::GetPanelWithID(const std::string& id)
	{
		auto result = m_IDPanels.find(id);
		if (result != m_IDPanels.end())
		{
			return result->second.get();
		}

		return nullptr;
	}

	bool PanelManager::OpenPanel(const std::string& id)
	{
		if (auto* panel = GetPanelWithID(id))
		{
			panel->OpenWindow(true);
			return true;
		}
		return false;
	}

	void PanelManager::OnGUI()
	{
		//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_None, &m_TopLevelClass);

		for (auto& panel : m_Panels)
		{
			if (panel->IsAsync() == false)
				panel->OnGUI();
		}

		for (auto& pair: m_IDPanels)
		{
			auto& panel = pair.second;
			if (panel->IsAsync() == false)
				panel->OnGUI();
		}
	}

	void PanelManager::TickAsyncUI()
	{
		for (auto& panel : m_Panels)
		{
			if (panel->IsAsync() == true)
				panel->OnGUI();
			//panel->OnGUI();
		}

		for (auto& pair : m_IDPanels)
		{
			auto& panel = pair.second;
			if (panel->IsAsync() == true)
				panel->OnGUI();
		}
	}

	void PanelManager::OnUpdate(float delta)
	{
		for (auto& panel : m_Panels)
		{
			panel->OnUpdate(delta);
		}

		for (auto& pair : m_IDPanels)
		{
			auto& panel = pair.second;
			panel->OnUpdate(delta);
		}
	}

	void PanelManager::OnFixedUpdate()
	{
		for (auto& panel : m_Panels)
		{
			panel->OnFixedUpdate();
		}

		for (auto& pair : m_IDPanels)
		{
			auto& panel = pair.second;
			panel->OnFixedUpdate();
		}
	}

	ImGuiWindowClass& PanelManager::GetImGuiWindowClass()
	{
		return m_TopLevelClass;
	}

	ImGuiWindow* PanelManager::GetWindow()
	{
		return ImGui::GetCurrentWindow();
	}

	ImGuiID PanelManager::GetWindowDockID()
	{
		return ImGui::GetID("TopLevelClass");
	}

	PanelManager* PanelManager::GetInstance()
	{
		static PanelManager* app = new PanelManager();

		return app;
	}
}