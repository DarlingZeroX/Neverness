#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../imgui/imgui.h"

namespace ImGuiEx
{
	struct IMGUI_API ImWindow
	{
		ImWindow();
		explicit ImWindow(const std::string& name);
		virtual  ~ImWindow() = default;

		virtual void OnGUI();

		//void CloseWindow();
		//void OpenWindow();
		void SetWindowNoClose();

		bool IsOpening() const noexcept;
		void SetWindowName(const std::string& name);

	protected:
		virtual void TickWindowMenuUI() {};
		virtual void OnWindowGUI() {  };
		virtual ImGuiWindowFlags GetWindowFlags() { return 0; }
	protected:
		std::string  m_WindowName;
		bool m_OpenState;
		bool* m_OpenStatePtr;
	};

	struct IMGUI_API IDockSpaceWindow : public ImWindow
	{
		IDockSpaceWindow();
		~IDockSpaceWindow() override = default;

		void InsertWindow(std::shared_ptr<ImWindow> window);

		std::vector<std::shared_ptr<ImWindow>>::iterator begin();
		std::vector<std::shared_ptr<ImWindow>>::iterator end();

	private:
		void OnWindowGUI() final override;
	protected:
		virtual void OnDockSpaceWindowGUI() = 0;
	private:
		std::vector<std::shared_ptr<ImWindow>> m_SubWindows;
		ImGuiWindowClass m_WindowClass;
	};
}
