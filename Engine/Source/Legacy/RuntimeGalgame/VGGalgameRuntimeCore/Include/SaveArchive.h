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
#include "../VGGRCExport.h"
#include <cstdint>
#include <VGRHI/Interface/Texture.h>
#include <NNKernel/Include/File/nlohmann/json.hpp>
#include "ArchiveDataContainer.h"

namespace VisionGal::GalGame
{
	struct VG_RUNTIME_GALCORE_API SaveArchive
	{
		/// Phase 7：顶层 JSON schema 整数版本（与 `version` 字符串独立演进）。
		static constexpr int kSaveArchiveSchemaVersion = 1;

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

		/// 预留：与 Choices/Inputs 命名空间内容校验相关（当前可为 0）。
		std::uint64_t schemaHash = 0;

		void WriteToJson(nlohmann::json& json);
		void ReadFromJson(nlohmann::json& json);

		/// 中文：反序列化前调用；不通过则 `isValid` 置 false，由调用方拒绝加载。
		static bool ValidateArchiveSchema(const nlohmann::json& root);
	};
}
