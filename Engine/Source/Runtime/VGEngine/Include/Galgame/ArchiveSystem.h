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
#include "Interface/GalgameInterface.h"
#include "../EngineConfig.h"
#include "VGCore/Include/Core/Core.h"
#include "Core/SaveArchive.h"
#include "Core/GalGameContext.h"
#include <HCore/Include/File/nlohmann/json.hpp>

namespace VisionGal::GalGame
{
	class VG_ENGINE_API ArchiveSystem: public IArchiveSystem
	{
	public:
		ArchiveSystem();
		ArchiveSystem(const ArchiveSystem&) = delete;
		ArchiveSystem& operator=(const ArchiveSystem&) = delete;
		ArchiveSystem(ArchiveSystem&&) noexcept = default;
		ArchiveSystem& operator=(ArchiveSystem&&) noexcept = default;
		~ArchiveSystem() override;

		bool Initialise(const Ref<GalGameContext>& ctx);

		SaveArchive SaveArchiveByNumber(const String& number);
		SaveArchive GetArchiveByNumber(const String& number);
		bool HasArchiveByNumber(const String& number);

		std::string GetCurrentDateFormat();
		std::string GetCurrentTimeFormat();
	private:
		void ReadArchives();
		void ReadArchive(const String& saveNumber, nlohmann::json& json);
	private:
		Ref<GalGameContext> m_GalGameContext;

		std::string m_ArchiveExt = ".save";
		std::filesystem::path m_BasePath;
		std::filesystem::path m_SaveDirectoryPath;

		//SaveArchive m_CurrentSaveArchiveState;

		std::unordered_map<String, SaveArchive> m_Archives;
	};
}
