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

#include "Asset/Accessor/SceneAccessor.h"
#include "Asset/Accessor/SceneSerializer.h"
#include "Asset/Accessor/SceneSerializeFormat.h"
#include <HCore/Include/Core/HSerialization.h>
#include "Core/VFS.h"
#include "Asset/Package.h"
#include "Scene/Components.h"

namespace VisionGal
{
	bool SceneAssetWriter::Write(const std::string path, VGAsset* asset)
	{
		SceneAsset* sceneAsset = dynamic_cast<SceneAsset*>(asset);
		Scene* scene = sceneAsset->WriteScene;

		// 创建或打开一个文件用于写入
		auto file = VFS::GetInstance()->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Write);
		if (file == nullptr)
			return false;

		if (!file->IsOpened())
			return false;

		SceneSerializeFormatHeader header;
		 
		// 1. 序列化对象到内存流
		std::stringstream serializedStream;
		{
			cereal::JSONOutputArchive archive(serializedStream);

			archive(cereal::make_nvp("VGSCENE_Magic", header.magic));
			archive(cereal::make_nvp("VGSCENE_MajorVersion", header.majorVersion));
			archive(cereal::make_nvp("VGSCENE_MinorVersion", header.minorVersion));
			archive(cereal::make_nvp("VGSCENE_Loader", header.loader));

			SceneSerializer serializer;
			serializer.SerializeScene(archive, scene);

		}
		// 2. 转换为输入流并写入 vfspp 文件
		std::istringstream inputStream(serializedStream.str());
		file->Write(inputStream, inputStream.str().size());
		file->Close();

		// 3. 写入元信息
		auto package = VGPackage::NewPackage(path);
		package->SetAsset(asset);
		package->WriteMetaData("");

		return true;
	}

	bool SceneAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		Ref<SceneAsset> sceneAsset = CreateRef<SceneAsset>();
		sceneAsset->LoadedScene = CreateRef<Scene>();

		IStringStreamVFS stream;
		if (stream.Open(path) == false)
			return false;
	//
		SceneSerializeFormatHeader currentVersionHeader;
		SceneSerializeFormatHeader fileHeader;
		bool isException = false;
		// 创建cereal归档器
		try {
			cereal::JSONInputArchive archive(stream.GetStream());

			archive(fileHeader.magic);
			archive(fileHeader.majorVersion);
			archive(fileHeader.minorVersion);
			archive(fileHeader.loader);

			if (fileHeader.magic != currentVersionHeader.magic)
			{
				H_LOG_ERROR( "错误场景文件" );
				isException = true;
			}
			else if (fileHeader.majorVersion > currentVersionHeader.majorVersion)
			{
				H_LOG_ERROR( "旧版引擎不支持新版场景资产" );
				isException = true;
			}

			SceneSerializer serializer;
			if (isException == false)
			{
				serializer.DeserializeScene(archive, sceneAsset->LoadedScene.get());
			}
		}
		catch (const cereal::RapidJSONException& e) {
			isException = true;
			H_LOG_ERROR( "JSON解析错误: %s", e.what() );
			// 输出当前JSON位置和内容，帮助调试
			// 可能需要使用cereal的调试API
		}
		catch (const cereal::Exception& e) {
			isException = true;
			H_LOG_ERROR( "序列化错误: %s", e.what() );
		}
		catch (const std::exception& e) {
			isException = true;
			H_LOG_ERROR( "标准异常: %s", e.what() );
		}

		if (isException)
		{
			asset = CreateRef<SceneAsset>();
			return false;
		}

		asset = sceneAsset;
		return true;
	}
}
