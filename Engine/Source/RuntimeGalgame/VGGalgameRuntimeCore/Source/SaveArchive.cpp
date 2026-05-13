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

#include "SaveArchive.h"

#include <cstdint>

namespace VisionGal::GalGame
{
	bool SaveArchive::ValidateArchiveSchema(const nlohmann::json& root)
	{
		if (!root.is_object())
			return false;
		const int schema = root.value("saveArchiveSchemaVersion", 1);
		if (schema < 1 || schema > kSaveArchiveSchemaVersion)
			return false;
		if (!root.contains("Base") || !root["Base"].is_object())
			return false;
		return true;
	}

	void SaveArchive::WriteToJson(nlohmann::json& json)
	{
		json["saveArchiveSchemaVersion"] = kSaveArchiveSchemaVersion;
		json["schemaHash"] = schemaHash;

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

		nlohmann::json data;
		if (archiveData)
		{
			archiveData->Serialize(data);
			data["archiveSchemaVersion"] = archiveData->schemaVersion;
			data["archiveSchemaHash"] = archiveData->schemaHash;
		}
		json["Data"] = data;

		json["contextSnapshot"] = nlohmann::json::object();
		json["runtimeState"] = nlohmann::json::object();
		json["presentationSnapshot"] = nlohmann::json::object();
	}

	void SaveArchive::ReadFromJson(nlohmann::json& json)
	{
		if (!ValidateArchiveSchema(json))
		{
			isValid = false;
			return;
		}

		schemaHash = json.value("schemaHash", std::uint64_t{0});

		auto base = json.value("Base", nlohmann::json({}));

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
			archiveData->schemaVersion = data.value("archiveSchemaVersion", 1);
			archiveData->schemaHash = data.value("archiveSchemaHash", std::uint64_t{0});
			archiveData->Deserialize(data);
		}

		(void)json.value("contextSnapshot", nlohmann::json::object());
		(void)json.value("runtimeState", nlohmann::json::object());
		(void)json.value("presentationSnapshot", nlohmann::json::object());
	}
}
