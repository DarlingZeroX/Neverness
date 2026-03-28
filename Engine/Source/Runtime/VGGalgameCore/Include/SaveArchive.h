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
#include "../VGGalCoreConfig.h"
#include <VGRHI/Interface/Texture.h>
#include <HCore/Include/File/nlohmann/json.hpp>
#include "ArchiveDataContainer.h"

namespace VisionGal::GalGame
{
	struct VG_GALGAME_CORE_API SaveArchive
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
		Ref<ArchiveDataContainer> archiveData = nullptr;

		void WriteToJson(nlohmann::json& json);
		void ReadFromJson(nlohmann::json& json);
	};
}
