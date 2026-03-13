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

#include "PakFormat.h"

namespace VisionGal {

	void PakEntry::Read(std::istream& stream)
	{
		stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
		stream.read(reinterpret_cast<char*>(&encryptionType), sizeof(encryptionType));
		stream.read(reinterpret_cast<char*>(&compressedType), sizeof(compressedType));
		stream.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
		stream.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
		stream.read(reinterpret_cast<char*>(&crc32), sizeof(crc32));
		uint16_t pathLen;
		stream.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
		path.resize(pathLen);
		stream.read(path.data(), pathLen);
	}

	void PakEntry::Write(std::ostream& stream)
	{
		stream.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		stream.write(reinterpret_cast<const char*>(&encryptionType), sizeof(encryptionType));
		stream.write(reinterpret_cast<const char*>(&compressedType), sizeof(compressedType));
		stream.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));
		stream.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));
		stream.write(reinterpret_cast<const char*>(&crc32), sizeof(crc32));
		uint16_t pathLen = static_cast<uint16_t>(path.size());
		stream.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
		stream.write(path.data(), pathLen);
	}

}
