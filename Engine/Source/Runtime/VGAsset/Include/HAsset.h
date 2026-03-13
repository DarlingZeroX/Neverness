// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once

#include <HCore/Interface/HCoreTypes.h>
#include <HCore/Include/Meta/Macros.h>
#include <HCore/Interface/HCore.h>
#include <HCore/Interface/HUUID.h>

#include <filesystem>

namespace Horizon
{
	struct HAssetMeatData
	{
		// 全局标识符，每个资产独有一个
		HUUID UUID;

		// 资产的类别，string类型
		std::string AssetType;

		HAssetMeatData() : UUID(0), AssetType("None") {}
		HAssetMeatData(std::string type) : UUID(0), AssetType(type) {}
	} ;
	 
	struct HAssetBase : public HObject
	{
		HAssetMeatData MetaData;

		std::filesystem::path Path;
		std::filesystem::path SourceFile;		// 资产的导入路径 Asset source disk file
		std::filesystem::path AssetPath;

		uint64 FilePathLength;
		uint64 FileSize;

		~HAssetBase() override = default;

		HAssetBase(const HAssetBase&) = default;
		HAssetBase& operator=(const HAssetBase&) = default;

		std::string GetAssetType() { return MetaData.AssetType; }

		HAssetBase() :
			MetaData("None"),
			Path("TestTest"),
			FilePathLength(0),
			FileSize(0)
		{

		}

		HAssetBase(std::string asset_type)
			: FilePathLength(0),
			FileSize(0),
			MetaData(asset_type)
		{
		}

		HAssetBase(uint64 uuid, const std::filesystem::path& path, uint64 file_path_length, uint64 file_size, std::string asset_type)
			: Path(path),
			FilePathLength(file_path_length),
			FileSize(file_size)
		{
		}
	};

}
