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
#include "Config.h"
#include "PakFormat.h"
#include <vector>
#include <fstream>
#include <filesystem>

namespace NN::Runtime {

	struct VG_PACKAGE_API PakFileReader {
		PakFileReader() = default;
		~PakFileReader() = default;

		bool ReadPakFile(const std::string& pakPath, const std::string& outPath, const std::string& aesKeyHex);

		static bool ReadEntryFile(const std::filesystem::path& path, const PakEntry& entry, std::vector<uint8_t>& outData);

		static bool ReadEntries(const std::filesystem::path& path, std::vector<PakEntry>& outEntries);

		std::ofstream m_OutFile;
		PakHeader m_Header;
		std::vector<PakEntry> m_Entries;

		//PackageFileWriter(const std::string& outPakPath, const std::string& aesKeyHex);
		//~PackageFileWriter();
		//
	};


}
