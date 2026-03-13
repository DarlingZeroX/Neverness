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

#pragma once
#include <VGAsset/Include/HAsset.h>
#include <VGImgui/IncludeImGuiEx.h>
#include <VGEngine/Include/Resource/Texture2D.h>

namespace VisionGal::Editor
{
	class TextureViewer : public ImGuiEx::ImTaskInterface
	{
		struct ImageViewData
		{
			float ImageWidth = 0.0f;
			float ImageHeight = 0.0f;
			float ImageViewWidth = 0.0f;
			float ImageViewHeight = 0.0f;
			float Scale = 1.0f;

			bool Channels[4];
		};

		struct ChannelButtonConfig
		{
			int ChannelIndex;
			ImVec4 ImGuiCol_Button;
			ImVec4 ImGuiCol_ButtonHovered;
			ImVec4 ImGuiCol_ButtonActive;
			ImVec2 ButtonSize;
			std::string label;
		};

		struct WindowData
		{
			ImVec2 WindowSize;
			ImVec2 WindowPos;
			ImDrawList* DrawList = nullptr;
		};
	public:
		TextureViewer(const VGPath& path);
		TextureViewer(const TextureViewer&) = default;
		TextureViewer& operator=(const TextureViewer&) = default;
		TextureViewer(TextureViewer&&) noexcept = default;
		TextureViewer& operator=(TextureViewer&&) noexcept = default;
		~TextureViewer() override;

		void RenderUI(TaskContext& context) override;
	private:
		void DrawTopMenu();
		void ColorChannelButton(const ChannelButtonConfig& config);

		void ScaleImage(float offset);
	private:
		VGPath m_Path;
		Ref<Texture2D> m_ImageTexture;

		//ImageViewProcessor m_ImageProcesser;
		ImageViewData m_ViewData;
		float m_ImageZoomSpeed = 0.05f;
		WindowData m_WindowData;
	};
}
