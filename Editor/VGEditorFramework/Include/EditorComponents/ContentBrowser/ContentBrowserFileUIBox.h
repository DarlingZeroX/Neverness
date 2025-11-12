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
#include "../../EditorCore/ContentBrowser.h"
#include <VGImgui/IncludeImGuiEx.h>

namespace VisionGal::Editor {

	struct ContentBrowserFileUIBox
	{
		ContentBrowserFileUIBox();

		static ContentBrowserFileUIBox& Get();

		void Draw(
			ContentBrowser* browser,
			ImDrawList* draw_list,
			ContentBrowserItem& item,
			const ImVec2& p0,
			const ImVec2& p1
		);

		float thumbnialTextSizeY;
		float padding;
		float thumbnailImageSizeY;

		float CellSize;
		ImVec2 Size;
		ImVec2 TextSize;
		ImVec2 ImageSize;
		ImU32 assetTypeColor;
	};

}