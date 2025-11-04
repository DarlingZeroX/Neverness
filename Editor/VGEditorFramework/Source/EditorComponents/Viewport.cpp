#include "EditorComponents/Viewport.h"
#include "VGImgui/Include/ImGuiEx/ImGuiEx.h"
#include <VGEngine/Include/UI/UISystem.h>
#include "EditorCore/Localization.h"

namespace VisionGal::Editor
{
	EditorViewport::EditorViewport(Viewport* viewport)
	{
		m_Viewport = viewport;
	}

	void EditorViewport::OnGUI()
	{
		if (!m_IsOpen)
			return;

		ImVec2 pos, size;

		if (ImGui::Begin(GetWindowFullName().c_str(), nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			//pos = ImGui::GetWindowPos() - ImGui::GetMainViewport()->Pos + ImGui::GetCursorPos();
			pos = ImGui::GetWindowPos() - ImGui::GetWindowViewport()->Pos + ImGui::GetCursorPos();
			size = ImGui::GetContentRegionAvail();

			auto* viewportTexture = m_Viewport->GetViewportTexture();
			if (viewportTexture != nullptr)
			{
				ImGuiEx::Image(viewportTexture->GetShaderResourceView(), size.x, size.y);
			}

			//H_LOG_INFO("Imgui X:%f, Y: %f", pos.x, pos.y);
			m_Viewport->SetViewportPosition({ pos.x, pos.y });
			// 确保视口尺寸是正确的
			if (size.x > 0 && size.y > 0)
				m_Viewport->SetViewportSize({ size.x,size.y });

			//auto viewport = ImGui::GetWindowViewport();
			//auto* window = static_cast<SDL_Window*>(ImGui::GetWindowViewport()->PlatformHandle);
			////auto windowID = SDL_GetWindowID(window);
			//H_LOG_INFO("window :%d", window);
			////m_Viewport->SetWindowID(windowID);

			// 这里存在问题,因为PlatformHandle获取的可能并不是SDL_Window*，而是SDL_WindowID
			ImGuiViewport* imViewport = ImGui::GetWindowViewport();
			SDL_WindowID windowID = reinterpret_cast<SDL_WindowID>(imViewport->PlatformHandle);
			//H_LOG_INFO("windowID X:%d", windowID);
			m_Viewport->SetWindowID(windowID);

			if (ImGui::IsWindowHovered())
			{
				//H_LOG_INFO("EnableInput");
				m_Viewport->EnableInput(true);
			}
			else
			{
				//H_LOG_INFO("DisableInput");
				m_Viewport->EnableInput(false);
			}
		}
		
		ImGui::End();
	}

	std::string EditorViewport::GetWindowFullName()
	{
		return EditorText{ GetWindowName(), ICON_FA_WINDOW }.GetText();
	}

	std::string EditorViewport::GetWindowName()
	{
		return "Main Viewport";
	}

	void EditorViewport::OpenWindow(bool open)
	{
		m_IsOpen = open;
	}

	bool EditorViewport::IsWindowOpened()
	{
		return m_IsOpen;
	}
}
