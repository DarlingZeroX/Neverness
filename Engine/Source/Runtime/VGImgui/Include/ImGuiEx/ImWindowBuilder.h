#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../imgui/imgui.h"

namespace ImGuiEx
{

	class ImWindowBuilder
	{
	public:

		void SetMenuBar(std::function<void()> callback);

		void PushLayer(std::function<void()> layer);

		void OnGUI();

	private:
		using GUI_CALLBACK = std::function<void()>;

		std::vector<GUI_CALLBACK> m_Layers;

		GUI_CALLBACK m_MenuBar;
		ImGuiWindowFlags m_WindowFlags;
	};
}
