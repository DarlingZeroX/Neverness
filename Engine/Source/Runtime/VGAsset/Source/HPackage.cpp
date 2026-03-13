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

//#include "pch.h"
#include "HPackage.h"
#include "HCore/Interface/HCoreTypes.h"
#include "HFileSystem/Interface/HFileSystem.h"
//#include "System/HFileSystem.h"

namespace Horizon
{
	HPackage::HPackage(HPath vpath)
	{
		string outDirectory;
		fsPath filePath = vpath.parent_path();
		string name;
		string ext;

		HFileSystem::SplitPath(vpath, &outDirectory, &name, &ext);

		auto fileFullName = outDirectory + name;

		m_VirtualDataFilePath = vpath;
		m_VirtualMetaFilePath = vpath.wstring() + L".gmeta";

		//m_PhysicalMetaFilePath = HVirtualFileSystem::HPathToPfsPath(vpath).wstring() + L".gmeta";
		//m_PhysicalDataFilePath = HVirtualFileSystem::HPathToPfsPath(vpath).wstring();
	}

	bool HPackage::GetMeatData(const HPath& path, HAssetMeatData& metadata)
	{
		// MetaData Stream
		wstring name = path.filename();
		fsPath parentPath = path.parent_path();
		fsPath metaPath = parentPath / (name + L".gmeta");

		// 打开Meta文件
		std::fstream m_MetaFile;
		m_MetaFile.open(metaPath, std::ios::in);

		if (!m_MetaFile.is_open())
			return false;

		// 读取文件第一行
		std::string metaData;
		getline(m_MetaFile, metaData);

		auto split = HStringTools::Split(metaData, ";");

		if (split.size() != 3)
			return false;

		// 标识符
		if (split[0].compare("[GASSET]") != 0)
			return false;

		// UUID
		auto& uuid = split[1];
		char* uuidEnd;
		metadata.UUID = std::strtoull(uuid.data(), &uuidEnd, 10);

		// 资产类别
		auto& assetType = split[2];
		char* assetTypeEnd;
		metadata.AssetType = assetType;

		m_MetaFile.close();

		return true;
	}

	Ref<HPackage> HPackage::NewPackage(const HPath& path)
	{
		std::string ext, name, parentPath;

		HFileSystem::SplitPath(path, &parentPath, &name, &ext);

		fsPath metaPath = parentPath;

		auto package = CreateRef<HPackage>(path);

		package->m_AssetMetaData.MetaData.UUID = HUUID::NewUUID();

		return package;
	}

	Ref<HPackage> HPackage::LoadPackage(const HPath& path, HResult& result)
	{
		auto package = CreateRef<HPackage>(path);

		package->Load();

		return package;
	}

	bool HPackage::CheckPackageValid(const HPath& path)
	{
		return true;
	}

	fsPath HPackage::GetAssetPath()
	{
		return m_PhysicalDataFilePath;
	}

	void HPackage::SetAssetType(std::string type)
	{
		m_AssetMetaData.MetaData.AssetType = type;
	}

	void HPackage::SetAsset(HAssetBase& asset)
	{
		asset.MetaData.UUID = m_AssetMetaData.MetaData.UUID;
		asset.Path = m_VirtualDataFilePath;
		m_AssetMetaData.MetaData.AssetType = asset.MetaData.AssetType;
	}

	void HPackage::OpenOutputStream()
	{
		m_AssetBinaryDataFile.open(m_PhysicalDataFilePath, std::ios::out | std::ios::binary);

		OpenOutputMetaStream();
	}

	void HPackage::CloseOutputStream()
	{
		// 关闭二进制文件流 BinaryData Stream
		m_AssetBinaryDataFile.close();

		CloseOutputMetaStream();
	}

	void HPackage::OpenOutputMetaStream()
	{
		m_AssetMetaJson.doc().SetObject();
	}

	void HPackage::CloseOutputMetaStream()
	{
		// 打开Meta流 MetaData Stream
		std::fstream metaStream;
		metaStream.open(m_PhysicalMetaFilePath, std::ios::out);

		// 写入GASSET资产的标识符，UUID,资产类别
		std::string metaData = "";
		metaData += "[GASSET];";
		metaData += m_AssetMetaData.MetaData.UUID.ToString();
		metaData += ";";
		metaData += m_AssetMetaData.MetaData.AssetType;
		metaData += "\n";

		metaStream.write(metaData.data(), metaData.size());

		m_AssetMetaJson.SaveJson(metaStream);

		metaStream.close();
	}

	void HPackage::OpenReadStream(HAssetBase& assetBase)
	{
		assetBase.Path = "";
		auto te = m_VirtualDataFilePath;

		assetBase.Path = m_VirtualDataFilePath;
		assetBase = m_AssetMetaData;
		assetBase.AssetPath = m_VirtualDataFilePath;

		m_AssetBinaryDataFile = std::fstream(m_PhysicalDataFilePath, std::ios::in | std::ios::binary);
	}

	void HPackage::CloseReadStream()
	{
		m_AssetBinaryDataFile.close();
	}

	void HPackage::ReadAsset(HAssetBase& assetBase)
	{
		assetBase.Path = m_VirtualDataFilePath;
		assetBase.AssetPath = m_VirtualDataFilePath;
	}

	void HPackage::OpenReadMetaStream()
	{

	}

	void HPackage::CloseReadMetaStream()
	{
	}

	HAssetMeatData HPackage::GetMeatData()
	{
		return m_AssetMetaData.MetaData;
	}

	HResult HPackage::Load()
	{
		// MetaData Stream
		std::fstream metaStream;
		metaStream.open(m_PhysicalMetaFilePath, std::ios::in);

		if (!metaStream.is_open())
			return false;

		std::string metaData;
		getline(metaStream, metaData);

		auto split = HStringTools::Split(metaData, ";");

		if (split.size() != 3)
			return false;

		if (split[0].compare("[GASSET]") != 0)
			return false;

		// UUID
		auto& uuid = split[1];
		char* uuidEnd;
		m_AssetMetaData.MetaData.UUID = std::strtoull(uuid.data(), &uuidEnd, 10);

		// 资产类别
		auto& assetType = split[2];
		m_AssetMetaData.MetaData.AssetType = assetType;

		m_AssetMetaJson.LoadJson(metaStream);

		metaStream.close();
	}
}
