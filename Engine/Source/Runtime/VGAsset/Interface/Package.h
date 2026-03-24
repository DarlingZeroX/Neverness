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
#include "../VGAssetConfig.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Interface/VGAsset.h"
#include <fstream>

namespace VisionGal
{
	struct VG_ASSET_API VGPackage
	{
		VGPackage() = default;
		VGPackage(const String& path);
		~VGPackage() = default;

		static Ref<VGPackage> NewPackage(const String& path);
		static Ref<VGPackage> LoadPackage(const String& path);

		static bool CheckPackageValid(const String& path);

		String GetAssetPath();
		//HJson& GetMetaJson() { return m_AssetMetaJson; };
		std::fstream& GetBinaryStream() { return m_AssetBinaryDataFile; }
		VGAssetMetaData GetMeatData();
		static bool GetMeatData(const String& path, VGAssetMetaData& metadata);
		static std::string GetAssetType(const String& path);
	public:
		void SetAssetType(const String& type);
		void SetAsset(VGAsset* asset);

		void OpenWriteStream();
		void CloseWriteStream();

		void OpenReadStream(VGAsset& assetBase);
		void CloseReadStream();

		bool WriteMetaData(const String& data);

		void ReadAsset(VGAsset& assetBase);

		void OpenReadMetaStream();
		void CloseReadMetaStream();

		String GetPhysicalDataFilePath() { return m_PhysicalDataFilePath; }
	private:
		bool Load();

		String m_VirtualMetaFilePath;
		String m_VirtualDataFilePath;

		//pfsPath m_PhysicalFilePath;
		String m_PhysicalMetaFilePath;
		String m_PhysicalDataFilePath;

		VGAssetMetaData m_AssetMetaData;

		std::fstream m_AssetBinaryDataFile;

		//std::stringstream m_MetaStream;
		//vfspp::IFilePtr m_OutMetaFile;

	};
}


