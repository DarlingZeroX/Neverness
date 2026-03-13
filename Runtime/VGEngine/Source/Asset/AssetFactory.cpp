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

#include "Asset/AssetFactory.h"
#include "Asset/GalGameAsset.h"
#include "Asset/LuaScriptAsset.h"
#include "Asset/Package.h"
#include "Asset/SceneAsset.h"
#include "Asset/UIAsset.h"
#include "Asset/Accessor/SceneAccessor.h"
#include "Core/VFS.h"
#include "Engine/Manager.h"
#include "HCorePlatform/Interface/HFileSystem.h"
#include "HCore/Include/Utils/HStringGenerator.h"
#include "Scene/GameActorFactory.h"

namespace VisionGal
{
	EngineAssetFactory::EngineAssetFactory()
	{
		m_AssetFactoryInstances.push_back(CreateScope<SceneAssetFactory>());
		m_AssetFactoryInstances.push_back(CreateScope<UIDocumentAssetFactory>());
		m_AssetFactoryInstances.push_back(CreateScope<UICssAssetFactory>());
		m_AssetFactoryInstances.push_back(CreateScope<LuaScriptAssetFactory>());
		m_AssetFactoryInstances.push_back(CreateScope<GalGameStoryScriptFactory>());
	}

	Ref<VGAsset> EngineAssetFactory::CreateAsset(const String& path, const String& type)
	{
		for (auto& factory: m_AssetFactoryInstances)
		{
			if (factory->GetFactoryType() == type)
			{
				return factory->CreateAsset(path);
			}
		}

		return nullptr;
	}

	VGPath IAssetFactoryInstance::GenerateAssetPath(const VGPath& path, const std::string& name, const std::string& ext)
	{
		Horizon::HSequenceStringGenerator gen(name);

		std::filesystem::path fsPath = path;
		auto nextName = gen.GetNext();
		auto fullPath = fsPath / (nextName + ext);
		while (Horizon::HFileSystem::ExistsDirectory(fullPath))
		{
			nextName = gen.GetNext();
			fullPath = fsPath / (nextName + ext);
		}

		return fullPath.string();
	}

	std::string SceneAssetFactory::GetFactoryType()
	{
		return "Scene";
	}

	Ref<VGAsset> SceneAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先得到保存路径
		auto aPath = GenerateAssetPath(absolutePath, "NewScene", ".vgasset");
		auto rPath = VFS::GetResourcePathVFS(aPath);

		// 创建场景资产
		SceneAssetWriter writer;
		Ref<Scene> scene = CreateRef<Scene>();
		GameActorFactory factory;
		factory.CreateActor(scene.get(), "Camera");
		factory.CreateActor(scene.get(), "Sprite");

		Ref<SceneAsset> asset = CreateRef<SceneAsset>();
		asset->WriteScene = scene.get();
		 
		// 序列化场景资产到本地
		if (writer.Write(rPath, asset.get()))
		{
			asset->LoadedScene = scene;
			return asset;
		}

		return nullptr;
	}

	std::string UIDocumentAssetFactory::GetFactoryType()
	{
		return "HTML";
	}

	Ref<VGAsset> UIDocumentAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先创建UI文档资产
		Ref<UIDocumentAsset> asset = CreateRef<UIDocumentAsset>();

		// 获取UI模版文档
		String templatePath = Core::GetEngineResourcePathVFS() + "asset/template/document.html";

		String templateText;
		if ( VFS::ReadTextFromFile(templatePath, templateText))
		{
			// 先得到保存路径
			auto aPath = GenerateAssetPath(absolutePath, "document", ".html");
			auto rPath = VFS::GetResourcePathVFS(aPath);

			// 写入资产数据
			asset->Text = templateText;
			UIDocumentAssetWriter writer;

			// 序列化UI文档资产到本地
			if (writer.Write(rPath, asset.get()))
			{
				return asset;
			}
		}

		return nullptr;
	}

	std::string UICssAssetFactory::GetFactoryType()
	{
		return "CSS";
	}

	Ref<VGAsset> UICssAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先创建UI文档资产
		Ref<UICssAsset> asset = CreateRef<UICssAsset>();

		// 获取UI模版文档
		String templatePath = Core::GetEngineResourcePathVFS() + "asset/template/style.css";
		String templateText;

		if (VFS::ReadTextFromFile(templatePath, templateText))
		{
			// 先得到保存路径
			auto aPath = GenerateAssetPath(absolutePath, "style", ".css");
			auto rPath = VFS::GetResourcePathVFS(aPath);

			// 写入资产数据
			asset->Text = templateText;
			UICssAssetWriter writer;

			// 序列化UI文档资产到本地
			if (writer.Write(rPath, asset.get()))
			{
				return asset;
			}
		}

		return nullptr;
	}

	std::string LuaScriptAssetFactory::GetFactoryType()
	{
		return "LuaScript";
	}

	Ref<VGAsset> LuaScriptAssetFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先创建UI文档资产
		Ref<LuaScriptAsset> asset = CreateRef<LuaScriptAsset>();

		// 获取UI模版文档
		String templatePath = Core::GetEngineResourcePathVFS() + "asset/template/luaScript.lua";
		String templateText;

		if (VFS::ReadTextFromFile(templatePath, templateText))
		{
			// 先得到保存路径
			auto aPath = GenerateAssetPath(absolutePath, "script", ".lua");
			auto rPath = VFS::GetResourcePathVFS(aPath);

			// 写入资产数据
			asset->Text = templateText;
			LuaScriptAssetWriter writer;

			// 序列化UI文档资产到本地
			if (writer.Write(rPath, asset.get()))
			{
				return asset;
			}
		}

		return nullptr;
	}

	std::string GalGameStoryScriptFactory::GetFactoryType()
	{
		return "GalGameStoryScript";
	}

	Ref<VGAsset> GalGameStoryScriptFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先创建UI文档资产
		Ref<GalGameStoryScriptAsset> asset = CreateRef<GalGameStoryScriptAsset>();

		// 获取UI模版文档
		String templatePath = Core::GetEngineResourcePathVFS() + "asset/template/galgameStoryScript.lua";
		String templateText;

		if (VFS::ReadTextFromFile(templatePath, templateText))
		{
			// 先得到保存路径
			auto aPath = GenerateAssetPath(absolutePath, "storyScript", ".lua");
			auto rPath = VFS::GetResourcePathVFS(aPath);

			// 写入资产数据
			asset->Text = templateText;
			GalGameStoryScriptAssetWriter writer;

			// 序列化UI文档资产到本地
			if (writer.Write(rPath, asset.get()))
			{
				return asset;
			}
		}

		return nullptr;
	}

	EngineAssetFactory* GetEngineAssetFactory()
	{
		static EngineAssetFactory factory;
		return &factory;
	}
}
