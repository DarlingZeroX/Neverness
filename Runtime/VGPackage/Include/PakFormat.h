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
#include <cstdint>
#include <string>
#include <istream>
#include <ostream>

namespace VisionGal {

	struct PakHeader {
		char magic[4] = { 'V','G','P','C' };  // 魔数
		uint32_t majorVersion = 0;
		uint32_t minorVersion = 1;
		uint64_t indexOffset = 0;
	};

	enum class PakCompressionType : uint32_t  // 压缩类型
	{
		None = 0,
		Zstd = 1,  // 使用 Zstd 压缩
		LZ4 = 2,   // 使用 LZ4 压缩
	};

	enum class PakEncryptionType : uint32_t  // 加密类型
	{
		None = 0,
		AES = 1,  // 使用 AES 加密
	};

	struct PakEntry {
		uint64_t offset = 0;
		PakEncryptionType encryptionType = PakEncryptionType::None;
		PakCompressionType compressedType = PakCompressionType::None;
		uint64_t compressedSize = 0;
		uint64_t originalSize = 0;
		uint32_t crc32 = 0;
		std::string path;

		void Read(std::istream& stream);
		void Write(std::ostream& stream);
	};
}
