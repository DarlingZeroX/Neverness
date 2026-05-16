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
#include <vector>
#include <cstdint>

namespace NN::Runtime {

	struct PakCrc32 {
		static uint32_t CalcCRC32(const std::vector<uint8_t>& data);
	private:
		 static void InitCrc32Table();
	};


}
