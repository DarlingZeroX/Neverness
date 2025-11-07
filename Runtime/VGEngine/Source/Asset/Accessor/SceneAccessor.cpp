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
#include <HCore/Include/Core/HSerialization.h>
#include "Asset/Package.h"
#include "Scene/Components.h"
#include "Asset/Accessor/SceneSerializer.h"
#include "Core/VFS.h"

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
		 
		// 1. 序列化对象到内存流
		std::stringstream serializedStream;
		{
			cereal::JSONOutputArchive archive(serializedStream);

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
		bool isException = false;
		// 创建cereal归档器
		try {
			cereal::JSONInputArchive archive(stream.GetStream());

			SceneSerializer serializer;
			serializer.DeserializeScene(archive, sceneAsset->LoadedScene.get());
		}
		catch (const cereal::RapidJSONException& e) {
			isException = true;
			std::cerr << "JSON解析错误: " << e.what() << std::endl;
			// 输出当前JSON位置和内容，帮助调试
			// 可能需要使用cereal的调试API
		}
		catch (const cereal::Exception& e) {
			isException = true;
			std::cerr << "序列化错误: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			isException = true;
			std::cerr << "标准异常: " << e.what() << std::endl;
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
