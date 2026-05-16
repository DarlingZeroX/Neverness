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

#include "PakReader.h"
#include "PakCrc32.h"
#include "NNKernel/Interface/HLog.h" // 添加日志头文件
#include <iostream>
#include <filesystem>

namespace VisionGal {
	namespace fs = std::filesystem;

	bool PakFileReader::ReadPakFile(const std::string& pakPath, const std::string& outPath, const std::string& aesKeyHex)
	{
		std::ifstream in(pakPath, std::ios::binary);
		if (!in) return false;

		PakHeader header;
		in.read(reinterpret_cast<char*>(&header), sizeof(header));

		in.seekg(header.indexOffset);
		uint32_t count;
		in.read(reinterpret_cast<char*>(&count), sizeof(count));

		std::vector<PakEntry> entries(count);
		for (auto& e : entries) {
			e.Read(in);
		}

		//std::vector<uint8_t> aesKey = HexToBytes(aesKeyHex);

		for (const auto& e : entries) {
			in.seekg(e.offset);
			std::vector<uint8_t> enc(e.compressedSize);
			in.read(reinterpret_cast<char*>(enc.data()), enc.size());

			//auto dec = DecryptAES(enc, aesKey);
			//auto raw = DecompressZstd(dec, e.originalSize);
			auto& raw = enc; // 假设没有压缩和加密，直接使用原始数据

			uint32_t crc = PakCrc32::CalcCRC32(raw);
			if (crc != e.crc32) {
				H_LOG_ERROR("CRC error on file: {}", e.path.c_str());
				continue;
			}

			auto path = std::filesystem::path(outPath) / e.path;
			//H_LOG_INFO("Out Extracted: %s", path.string().c_str());

			fs::path dir = path.parent_path();

			// 创建目录（如果不存在）
			if (!dir.empty() && !fs::exists(dir)) {
				fs::create_directories(dir);
			}

			std::ofstream outFile(path, std::ios::binary);
			outFile.write(reinterpret_cast<char*>(raw.data()), raw.size());
			H_LOG_INFO("Extracted: %s", e.path.c_str());
		}

		return true;
	}

	bool PakFileReader::ReadEntryFile(const std::filesystem::path& path, const PakEntry& entry,
		std::vector<uint8_t>& outData)
	{
		std::ifstream in(path, std::ios::binary);
		if (!in) {
			H_LOG_ERROR("Failed to open pak file: {}", path.string().c_str());
			return false;
		}

		if (entry.offset == 0) {
			H_LOG_ERROR("Entry offset is zero: {}", entry.path.c_str());
			return false;
		}

		if (entry.compressedSize == 0) {
			H_LOG_ERROR("Entry is empty: {}", entry.path.c_str());
			return false;
		}

		in.seekg(entry.offset);
		outData.resize(entry.compressedSize);
		in.read(reinterpret_cast<char*>(outData.data()), entry.compressedSize);

		if (in.gcount() != entry.compressedSize) {
			H_LOG_ERROR("Failed to read entry data from pak file: {}", path.string().c_str());
			return false;
		}

		if (outData.empty()) {
			H_LOG_ERROR("Entry data is empty: {}", entry.path.c_str());
			return false;
		}

		//auto dec = DecryptAES(outData, aesKey);
		//auto raw = DecompressZstd(dec, e.originalSize);
		//outData = raw; 

		// 假设没有压缩和加密，直接使用原始数据
		uint32_t crc = PakCrc32::CalcCRC32(outData);
		if (crc != entry.crc32) {
			H_LOG_ERROR("CRC error on entry: {}", entry.path.c_str());
			return false;
		}
		return true;
	}

	bool PakFileReader::ReadEntries(const std::filesystem::path& path, std::vector<PakEntry>& outEntries)
	{
		std::ifstream in(path, std::ios::binary);
		if (!in) return false;

		PakHeader header;
		in.read(reinterpret_cast<char*>(&header), sizeof(header));

		in.seekg(header.indexOffset);
		uint32_t count;
		in.read(reinterpret_cast<char*>(&count), sizeof(count));

		//std::vector<PakEntry> entries(count);
		outEntries.resize(count);
		for (auto& e : outEntries) {
			e.Read(in);
		}

		in.close();

		return true;
	}
}
