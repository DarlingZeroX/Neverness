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
#include "../Include/Core/Core.h"

namespace VisionGal
{
	struct VGAssetType
	{
		virtual ~VGAssetType() = default;
		virtual std::string GetNameID() const = 0;
	};

	struct VGAssetMetaData
	{
		VGAssetMetaData() : UUID(0), AssetType("None") {}
		VGAssetMetaData(std::string type) : UUID(0), AssetType(type) {}

		// 全局标识符，每个资产独有一个
		Horizon::HUUID UUID;

		// 资产的类别，string类型
		std::string AssetType;
	};

	struct VGAsset
	{
		VGAsset(const VGAsset&) = default;
		VGAsset& operator=(const VGAsset&) = default;
		VGAsset(VGAsset&&) noexcept = default;
		VGAsset& operator=(VGAsset&&) noexcept = default;
		virtual ~VGAsset() = default;
		VGAsset() :
			MetaData("None"), FilePathLength(0), FileSize(0)
		{
		}

		VGAsset(const String& asset_type)
			: FilePathLength(0), FileSize(0), MetaData(asset_type)
		{
		}

		VGAsset(const VGAssetType& assetType)
			: FilePathLength(0), FileSize(0), MetaData(assetType.GetNameID())
		{
		}

		VGAsset(uint64 uuid, const VGPath& path, uint64 file_path_length, uint64 file_size, const String& asset_type)
			: Path(path), FilePathLength(file_path_length), FileSize(file_size)
		{
		}

		std::string GetAssetType() { return MetaData.AssetType; }

		VGAssetMetaData MetaData;

		VGPath Path;
		VGPath SourceFile;		// 资产的导入路径 Asset source disk file
		VGPath AssetPath;

		uint64 FilePathLength;
		uint64 FileSize;
	};

	//struct VGAsset : public Horizon::HAssetBase
	//{
	//    bool IsGalFile = false;
	//
	//	VGAsset() = default;
	//	VGAsset(std::string asset_type): HAssetBase(asset_type) {}
	//
	//    ~VGAsset() override = default;
	//};

	struct IAssetWriter
	{
		virtual ~IAssetWriter() = default;

		virtual bool Write(const std::string path, VGAsset* asset) = 0;
	};

	struct IAssetLoader
	{
		virtual ~IAssetLoader() = default;

		virtual bool Read(const std::string path, Ref<VGAsset>& asset) = 0;
	};
}