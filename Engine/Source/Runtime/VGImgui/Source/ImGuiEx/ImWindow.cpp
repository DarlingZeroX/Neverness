#include "pch.h"
#include "ImGuiEx/ImWindow.h"

#include "imgui/imgui_internal.h"

namespace ImGuiEx
{
	ImWindow::ImWindow()
		:
		m_OpenState(true)
		, m_OpenStatePtr(nullptr)
	{
	}

	ImWindow::ImWindow(const std::string& name)
		:m_WindowName(name)
	{
	}

	void ImWindow::OnGUI()
	{
		const auto flags = GetWindowFlags();

		if (ImGui::Begin(m_WindowName.c_str(), m_OpenStatePtr, flags))
		{
			if(flags & ImGuiWindowFlags_MenuBar)
			{
				if (ImGui::BeginMenuBar())
				{
					TickWindowMenuUI();
					ImGui::EndMenuBar();
				}
			}

			OnWindowGUI();
		}
		ImGui::End();
	}

	//void ImWindow::CloseWindow()
	//{
	//	m_OpenState = false;
	//}
	//
	//void ImWindow::OpenWindow()
	//{
	//	m_OpenState = true;
	//}

	void ImWindow::SetWindowNoClose()
	{
		m_OpenStatePtr = nullptr;
	}

	bool ImWindow::IsOpening() const noexcept
	{
		return m_OpenState;
	}

	void ImWindow::SetWindowName(const std::string& name)
	{
		m_WindowName = name;
	}

	IDockSpaceWindow::IDockSpaceWindow()
	{
		m_WindowClass.ClassId = ImHashStr("I Dock Space Window Test");
		m_WindowClass.DockingAllowUnclassed = false;
	}

	void IDockSpaceWindow::InsertWindow(std::shared_ptr<ImWindow> window)
	{
		m_SubWindows.push_back(window);
	}

	void IDockSpaceWindow::OnWindowGUI()
	{
		OnDockSpaceWindowGUI();

		ImGui::DockSpace(m_WindowClass.ClassId, ImVec2(0, 0), 0, &m_WindowClass);

		for (auto& ptr : m_SubWindows)
		{
			ImGui::SetNextWindowClass(&m_WindowClass);
			ptr->OnGUI();
		}
	}

	std::vector<std::shared_ptr<ImWindow>>::iterator IDockSpaceWindow::begin()
	{
		return m_SubWindows.begin();
	}

	std::vector<std::shared_ptr<ImWindow>>::iterator IDockSpaceWindow::end()
	{
		return m_SubWindows.end();
	}
}