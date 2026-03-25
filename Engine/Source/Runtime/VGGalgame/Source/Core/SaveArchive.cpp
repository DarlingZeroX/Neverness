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

#include "Core/SaveArchive.h"

namespace VisionGal::GalGame
{
	void SaveArchive::WriteToJson(nlohmann::json& json)
	{
		// 序列化到json
		json["Base"]["IsGalGameArchive"] = isGalGameArchive;
		json["Base"]["Version"] = version;
		json["Base"]["ScriptPath"] = scriptPath;
		json["Base"]["Line"] = line;

		json["Base"]["SaveNumberString"] = saveNumberString;
		json["Base"]["Date"] = date;
		json["Base"]["Time"] = time;
		json["Base"]["DateTime"] = dateTime;
		json["Base"]["Description"] = description;
		json["Base"]["ScreenshotPath"] = screenshotPath;

		// 序列化存档数据
		nlohmann::json data;
		archiveData->Serialize(data);
		json["Data"] = data;
	}

	void SaveArchive::ReadFromJson(nlohmann::json& json)
	{
		auto base = json.value("Base", nlohmann::json({}));

		// 反序列化本地数据到存档
		isValid = false;
		isGalGameArchive = base.value("IsGalGameArchive", false);
		version = base.value("Version", "");
		scriptPath = base.value("ScriptPath", "");
		line = base.value("Line", 0);

		saveNumberString = base.value("SaveNumberString", "");
		date = base.value("Date", "");
		time = base.value("Time", "");
		dateTime = base.value("DateTime", "");
		description = base.value("Description", "");
		screenshotPath = base.value("ScreenshotPath", "");

		isValid = isGalGameArchive;

		if (json.contains("Data") && json["Data"].is_object())
		{
			auto& data = json["Data"];
			archiveData = MakeRef<ArchiveDataContainer>();
			archiveData->Deserialize(data);
		}
	}
}
