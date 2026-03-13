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

#include "Asset/Package.h"

#include "Core/VFS.h"
#include "HCore/Include/Core/HStringTools.h"
#include "HCorePlatform/Interface/HFileSystem.h"

namespace VisionGal
{
	VGPackage::VGPackage(const String& path)
	{
		m_VirtualDataFilePath = path;
		m_VirtualMetaFilePath = path + ".meta";

		m_PhysicalMetaFilePath = VFS::GetInstance()->AbsolutePath(m_VirtualDataFilePath) + ".meta";
		m_PhysicalDataFilePath = VFS::GetInstance()->AbsolutePath(m_VirtualDataFilePath);
	}

	bool VGPackage::GetMeatData(const String& path, VGAssetMetaData& metadata)
	{
		std::string outDirectory;
		std::string name;
		std::string ext;

		Horizon::HFileSystem::SplitPath(path, &outDirectory, &name, &ext);

		// MetaData Stream
		String metaPath = path + ".meta";

		IStringStreamVFS stream;
		if (!stream.Open(metaPath))
		{
			if (ext == ".png" || ext == ".jpg" || ext == ".bmp" || ext == ".tga")
			{
				metadata.AssetType = "Texture";
				return true;
			}
			if (ext == ".lua")
			{
				metadata.AssetType = "LuaScript";
				return true;
			}
			if (ext == ".html")
			{
				metadata.AssetType = "HTML";
				return true;
			}
			if (ext == ".css")
			{
				metadata.AssetType = "CSS";
				return true;
			}
			if (ext == ".ttf" || ext == ".ttc" || ext == ".otf")
			{
				metadata.AssetType = "Font";
				return true;
			}
			if (ext == ".mp3" || ext == ".wav" || ext == ".flac" || ext == ".m4a" || ext == ".ogg" )
			{
				metadata.AssetType = "Sound";
				return true;
			}
			if (ext == ".mp4" || ext == ".avi" || ext == ".mkv" || ext == ".mov" || ext == ".webm" || ext == ".flv")
			{
				metadata.AssetType = "Video";
				return true;
			}

			return false;
		}

		// 读取文件第一行
		std::string metaData;
		getline(stream.GetStream(), metaData);

		auto split = Horizon::HStringTools::Split(metaData, ";");

		if (split.size() != 3)
			return false;

		// 标识符
		if (split[0].compare("[VGASSET]") != 0)
			return false;

		// UUID
		auto& uuid = split[1];
		char* uuidEnd;
		metadata.UUID = std::strtoull(uuid.data(), &uuidEnd, 10);

		// 资产类别
		auto& assetType = split[2];
		char* assetTypeEnd;
		metadata.AssetType = assetType;

		return true;
	}

	std::string VGPackage::GetAssetType(const String& path)
	{
		VGAssetMetaData metadata;
		if (GetMeatData(path, metadata))
		{
			return metadata.AssetType;
		}
		return "None";
	}

	Ref<VGPackage> VGPackage::NewPackage(const String& path)
	{
		//String ext, name, parentPath;
		//Horizon::HFileSystem::SplitPath(path.c_str(), &parentPath, &name, &ext);

		auto package = CreateRef<VGPackage>(path);
		package->m_AssetMetaData.UUID = Horizon::HUUID::NewUUID();
		return package;
	}

	Ref<VGPackage> VGPackage::LoadPackage(const String& path)
	{
		auto package = CreateRef<VGPackage>(path);

		package->Load();

		return package;
	}

	bool VGPackage::CheckPackageValid(const String& path)
	{
		return true;
	}

	String VGPackage::GetAssetPath()
	{
		return m_PhysicalDataFilePath;
	}

	void VGPackage::SetAssetType(const String& type)
	{
		m_AssetMetaData.AssetType = type;
	}

	void VGPackage::SetAsset(VGAsset* asset)
	{
		asset->MetaData.UUID = m_AssetMetaData.UUID;
		asset->Path = m_VirtualDataFilePath;
		m_AssetMetaData.AssetType = asset->MetaData.AssetType;
	}

	void VGPackage::OpenWriteStream()
	{
		m_AssetBinaryDataFile.open(m_PhysicalDataFilePath, std::ios::out | std::ios::binary);

		//OpenOutputMetaStream();
	}

	void VGPackage::CloseWriteStream()
	{
		// 关闭二进制文件流 BinaryData Stream
		m_AssetBinaryDataFile.close();

		//CloseOutputMetaStream();
	}

	bool VGPackage::WriteMetaData(const String& data)
	{
		// 创建或打开一个文件用于写入
		auto metaFile = VFS::GetInstance()->OpenFile(vfspp::FileInfo(m_VirtualMetaFilePath), vfspp::IFile::FileMode::Write);
		if (!metaFile->IsOpened())
			return false;

		// 写入GASSET资产的标识符，UUID,资产类别
		std::string metaData = "";
		metaData += "[VGASSET];";
		metaData += m_AssetMetaData.UUID.ToString();
		metaData += ";";
		metaData += m_AssetMetaData.AssetType;
		metaData += "\n";

		std::stringstream metaStream;
		metaStream.write(metaData.data(), metaData.size());
		metaStream.write(data.data(), data.size());

		std::istringstream inputStream(metaStream.str());
		metaFile->Write(inputStream, inputStream.str().size());
		metaFile->Close();

		return true;
	}

	void VGPackage::OpenReadStream(VGAsset& assetBase)
	{
		//assetBase.Path = m_VirtualDataFilePath;
		//assetBase = m_AssetMetaData;
		//assetBase.AssetPath = m_VirtualDataFilePath;
		//
		//m_AssetBinaryDataFile = std::fstream(m_PhysicalDataFilePath, std::ios::in | std::ios::binary);
	}

	void VGPackage::CloseReadStream()
	{
		m_AssetBinaryDataFile.close();
	}

	void VGPackage::ReadAsset(VGAsset& assetBase)
	{
		assetBase.Path = m_VirtualDataFilePath;
		assetBase.AssetPath = m_VirtualDataFilePath;
	}

	void VGPackage::OpenReadMetaStream()
	{

	}

	void VGPackage::CloseReadMetaStream()
	{
	}

	VGAssetMetaData VGPackage::GetMeatData()
	{
		return m_AssetMetaData;
	}

	bool VGPackage::Load()
	{
		GetMeatData(m_VirtualDataFilePath, m_AssetMetaData);

		// MetaData Stream
	//	std::fstream metaStream;
	//	metaStream.open(m_PhysicalMetaFilePath, std::ios::in);
	//
	//	if (!metaStream.is_open())
	//		return false;
	//
	//	std::string metaData;
	//	getline(metaStream, metaData);
	//
	//	auto split = Horizon::HStringTools::Split(metaData, ";");
	//
	//	if (split.size() != 3)
	//		return false;
	//
	//	if (split[0].compare("[VGASSET]") != 0)
	//		return false;
	//
	//	// UUID
	//	auto& uuid = split[1];
	//	char* uuidEnd;
	//	m_AssetMetaData.UUID = std::strtoull(uuid.data(), &uuidEnd, 10);
	//
	//	// 资产类别
	//	auto& assetType = split[2];
	//	m_AssetMetaData.AssetType = assetType;
	//
	//	//m_AssetMetaJson.LoadJson(metaStream);
	//
	//	metaStream.close();

		return true;
	}

}
