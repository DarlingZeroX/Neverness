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
#include "Interface.h"
#include "../EngineConfig.h"
#include "../Core/Core.h"
#include <HCore/Include/System/HFileSystem.h>
#include "HCore/Include/File/nlohmann/json.hpp"

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

	class VG_ENGINE_API ArchiveSystem: public IArchiveSystem
	{
	public:
		ArchiveSystem();
		ArchiveSystem(const ArchiveSystem&) = delete;
		ArchiveSystem& operator=(const ArchiveSystem&) = delete;
		ArchiveSystem(ArchiveSystem&&) noexcept = default;
		ArchiveSystem& operator=(ArchiveSystem&&) noexcept = default;
		~ArchiveSystem() override;

		void UpdateSaveArchive(const SaveArchive& archive);

		bool Initialise();

		SaveArchive SaveArchiveByNumber(const String& number);
		SaveArchive GetArchiveByNumber(const String& number);
		bool HasArchiveByNumber(const String& number);

		std::string GetCurrentDateFormat();
		std::string GetCurrentTimeFormat();
	private:
		void ReadArchives();
		void ReadArchive(const String& saveNumber, nlohmann::json& json);
	private:
		std::string m_ArchiveExt = ".save";
		std::filesystem::path m_BasePath;
		std::filesystem::path m_SaveDirectoryPath;

		SaveArchive m_CurrentSaveArchiveState;

		std::unordered_map<String, SaveArchive> m_Archives;
	};
}
