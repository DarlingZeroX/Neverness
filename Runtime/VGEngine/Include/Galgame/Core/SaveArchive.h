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
#include "../Interface/GalgameInterface.h"
#include <VGRHI/Interface/Texture.h>

namespace VisionGal::GalGame
{
	struct SaveArchive
	{
		bool isGalGameArchive = true;
		bool isValid = true;
		String version = "0.1";
		String scriptPath;
		uint line;

		String saveNumberString;
		String date;
		String time;
		String dateTime;
		String description;
		String screenshotPath;

		Ref<VGFX::TexturePixels> screenshotPixels = nullptr;
	};
}
