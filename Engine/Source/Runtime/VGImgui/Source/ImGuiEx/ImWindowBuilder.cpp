#include "pch.h"
#include "ImGuiEx/ImWindowBuilder.h"

namespace ImGuiEx
{

	void ImWindowBuilder::SetMenuBar(std::function<void()> callback)
	{
		m_MenuBar = callback;
		
	}

	void ImWindowBuilder::PushLayer(std::function<void()> layer)
	{
		m_Layers.push_back(layer);
	}

	void ImWindowBuilder::OnGUI()
	{
		if(m_MenuBar)
		{
			m_WindowFlags |= ImGuiWindowFlags_MenuBar;
		}

		if (ImGui::Begin("Material Editor", nullptr, m_WindowFlags))
		{
			if (m_MenuBar)
			{
				m_MenuBar();
			}

			for (auto& layer : m_Layers)
			{
				layer();
			}
		}

		ImGui::End();
	}
}
