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

#include "AssetEditor/TextureViewer.h"
#include <NNRuntimeImGui/IncludeImGuiEx.h>
#include <NNRuntimeCore/Interface/Loader.h>

#include "NNEditorFramework/Include/EditorCore/Localization.h"
#include <NNKernel/Interface/HVector.h>

namespace VisionGal::Editor
{
	TextureViewer::TextureViewer(const VGPath& path)
	{
		m_Path = path;
		m_ImageTexture = LoadObject<Texture2D>(path);

		for (bool& c : m_ViewData.Channels)
			c = true;
	}

	TextureViewer::~TextureViewer()
	{
	}

	void TextureViewer::RenderUI(TaskContext& context)
	{
		ImGuiEx::ScopedStyleVar winBorder(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGuiEx::ScopedStyleColor winbg(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.0f));

		if (!m_ImageTexture || !m_ImageTexture->GetTexture())
			return;

		auto image = m_ImageTexture->GetTexture();

		//auto image = m_ImageProcesser.GetProcessedImage();
		auto& imageDesc = image->GetDesc();
		m_ViewData.ImageWidth = imageDesc.Width;
		m_ViewData.ImageHeight = imageDesc.Height;
		m_ViewData.ImageViewWidth = m_ViewData.Scale * m_ViewData.ImageWidth;
		m_ViewData.ImageViewHeight = m_ViewData.Scale * m_ViewData.ImageHeight;

		// Handle window resize 
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX),
			[](ImGuiSizeCallbackData* data)
			{
				ImageViewData* viewData = static_cast<ImageViewData*>(data->UserData);

				// Clamp the minimum size
				float minSizeY = 300.f;
				float minSizeX = minSizeY * (viewData->ImageHeight / viewData->ImageWidth);

				if (data->DesiredSize.x < minSizeX)
					data->DesiredSize.x = minSizeX;
				if (data->DesiredSize.y < minSizeY)
					data->DesiredSize.y = minSizeY;
			}
		, &m_ViewData);

		ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);

		bool open = true;
		// 让每个窗口都有独立的窗口ID
		EditorText text(m_Path, ICON_FA_IMAGE);
		std::string windowName = text.GetText() + "##" + std::to_string(context.Index);
		if (ImGui::Begin(windowName.c_str(), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar))
		{
			ImGuiEx::ScopedID winID(context.Index);

			m_WindowData.DrawList = ImGui::GetWindowDrawList();
			m_WindowData.WindowSize = ImGui::GetWindowSize();
			m_WindowData.WindowPos = ImGui::GetWindowPos();

			DrawTopMenu();

			//Image Process
			//m_ImageProcesser.Render();

			// Scale Image 
			const bool isHover = ImGui::IsWindowHovered();
			const bool isFocused = ImGui::IsWindowFocused();
			if (isHover && isFocused)
			{
				if (ImGui::GetIO().MouseWheel > 0.00001f)
					ScaleImage(0.1f);
				else if (ImGui::GetIO().MouseWheel < -0.00001f)
					ScaleImage(-0.1f);
			}

			// Compute start position Image display
			float2 ImagePos = { m_WindowData.WindowSize.x, m_WindowData.WindowSize.y };
			ImagePos = (ImagePos - float2{ m_ViewData.ImageViewWidth ,m_ViewData.ImageViewHeight }) / 2.0f;
			ImagePos = glm::max(ImagePos, float2(0.f));
			ImGui::SetCursorPos({ ImagePos.x, ImagePos.y });
			ImGuiEx::ImageGL(image->GetShaderResourceView(), m_ViewData.ImageViewWidth, m_ViewData.ImageViewHeight);

			// Image bounding rectangle
			const float thickness = 2.0f;
			const ImVec2 imageBgStart = ImVec2(m_WindowData.WindowPos.x + ImagePos.x, m_WindowData.WindowPos.y + ImagePos.y);
			m_WindowData.DrawList->AddRect(
				imageBgStart,
				ImVec2(imageBgStart.x + m_ViewData.ImageViewWidth, imageBgStart.y + m_ViewData.ImageViewHeight),
				IM_COL32(100, 100, 100, 150), 4.0f, 0, thickness
			);
		}

		ImGui::End();

		if (!open)
			context.IsFinished = true;
	}

	void DisabledChannelColor()
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
	}

	void TextureViewer::ColorChannelButton(const ChannelButtonConfig& config)
	{
		if (m_ViewData.Channels[config.ChannelIndex])
		{
			ImGui::PushStyleColor(ImGuiCol_Button, config.ImGuiCol_Button);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, config.ImGuiCol_ButtonHovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, config.ImGuiCol_ButtonActive);
		}
		else
		{
			DisabledChannelColor();
		}

		if (ImGui::Button(config.label.c_str(), config.ButtonSize))
		{
			m_ViewData.Channels[config.ChannelIndex] = !m_ViewData.Channels[config.ChannelIndex];
			//m_ImageProcesser.SetChannelVisibility(config.ChannelIndex, m_ViewData.Channels[config.ChannelIndex]);
		}
		ImGui::PopStyleColor(3);
	}

	void TextureViewer::DrawTopMenu()
	{
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		if (!ImGui::BeginMenuBar())
			return;

		ChannelButtonConfig red;
		red.ChannelIndex = 0;
		red.ImGuiCol_Button = { 0.8f, 0.1f, 0.15f, 1.0f };
		red.ImGuiCol_ButtonHovered = { 0.9f, 0.2f, 0.2f, 1.0f };
		red.ImGuiCol_ButtonActive = { 0.8f, 0.1f, 0.15f, 1.0f };
		red.ButtonSize = buttonSize;
		red.label = "R";
		ColorChannelButton(red);
		//ImGui::SameLine();

		ChannelButtonConfig green;
		green.ChannelIndex = 1;
		green.ImGuiCol_Button = { 0.2f, 0.7f, 0.2f, 1.0f };
		green.ImGuiCol_ButtonHovered = { 0.3f, 0.8f, 0.3f, 1.0f };
		green.ImGuiCol_ButtonActive = { 0.2f, 0.7f, 0.2f, 1.0f };
		green.ButtonSize = buttonSize;
		green.label = "G";
		ColorChannelButton(green);
		//ImGui::SameLine();

		ChannelButtonConfig blue;
		blue.ChannelIndex = 2;
		blue.ImGuiCol_Button = { 0.1f, 0.25f, 0.8f, 1.0f };
		blue.ImGuiCol_ButtonHovered = { 0.2f, 0.35f, 0.9f, 1.0f };
		blue.ImGuiCol_ButtonActive = { 0.1f, 0.25f, 0.8f, 1.0f };
		blue.ButtonSize = buttonSize;
		blue.label = "B";
		ColorChannelButton(blue);
		//ImGui::SameLine();

		ChannelButtonConfig alpha;
		alpha.ChannelIndex = 3;
		alpha.ImGuiCol_Button = { 0.6f, 0.6f, 0.6f, 1.0f };
		alpha.ImGuiCol_ButtonHovered = { 0.7f, 0.7f, 0.7f, 1.0f };
		alpha.ImGuiCol_ButtonActive = { 0.5f, 0.5f, 0.5f, 1.0f };
		alpha.ButtonSize = buttonSize;
		alpha.label = "A";
		ColorChannelButton(alpha);
		//ImGui::SameLine(0.0f, 5.0f);

		ImGui::Separator();

		ImGui::Text("Zoom");
		ImGui::SetNextItemWidth(200);
		int zoomV = m_ViewData.Scale * 100;
		ImGui::SliderInt("##Image Zoom", &zoomV, 1, 1600);
		m_ViewData.Scale = zoomV / 100.0f;

		ImGui::Separator();

		ImGui::EndMenuBar();
	}

	void TextureViewer::ScaleImage(float offset)
	{
		m_ViewData.Scale += offset;
		m_ViewData.Scale = std::max(m_ViewData.Scale, 0.1f);
	}
}
