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

#include "AssetFactory.h"
#include "VGAsset/Include/GalGameAsset.h"
#include "VGAsset/Include/LuaScriptAsset.h"
#include "VGAsset/Interface/Package.h"
#include "VGAsset/Include/SceneAsset.h"
#include "VGAsset/Include/UIAsset.h"
#include "VGAsset/Interface/SceneAccessor.h"
#include "VGCore/Include/Core/VFS.h"
#include "HFileSystem/Interface/HFileSystem.h"
#include "VGCore/Interface/ISceneFactory.h"

namespace VisionGal
{
	EngineAssetFactory::EngineAssetFactory()
	{
		m_AssetFactoryInstances.push_back(MakeScope<SceneAssetFactory>());
		m_AssetFactoryInstances.push_back(MakeScope<UIDocumentAssetFactory>());
		m_AssetFactoryInstances.push_back(MakeScope<UICssAssetFactory>());
		m_AssetFactoryInstances.push_back(MakeScope<LuaScriptAssetFactory>());
		m_AssetFactoryInstances.push_back(MakeScope<GalGameStoryScriptFactory>());
	}

	EngineAssetFactory& EngineAssetFactory::Get()
	{
		static EngineAssetFactory factory;
		return factory;
	}

	void EngineAssetFactory::RegisterFactory(Scope<IAssetFactoryInstance> factoryInstance)
	{
		m_AssetFactoryInstances.push_back(std::move(factoryInstance));
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
		Ref<IScene> scene = SceneFactoryRegistry::GetFactory()->CreateScene();
		auto factory = GameActorFactoryRegistry::GetFactory();
		factory->CreateActor(scene.get(), "Camera");
		factory->CreateActor(scene.get(), "Sprite");

		Ref<SceneAsset> asset = MakeRef<SceneAsset>();
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
		Ref<UIDocumentAsset> asset = MakeRef<UIDocumentAsset>();

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
		Ref<UICssAsset> asset = MakeRef<UICssAsset>();

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
		Ref<LuaScriptAsset> asset = MakeRef<LuaScriptAsset>();

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
		return GLuaScriptAssetType{}.GetNameID();
	}

	Ref<VGAsset> GalGameStoryScriptFactory::CreateAsset(const String& path)
	{
		auto absolutePath = VFS::GetInstance()->AbsolutePath(path);

		// 先创建UI文档资产
		Ref<GalGameLuaScriptAsset> asset = MakeRef<GalGameLuaScriptAsset>();

		// 获取UI模版文档
		String templatePath = Core::GetEngineResourcePathVFS() + "asset/template/galgameStoryScript.lua";
		String templateText;

		if (VFS::ReadTextFromFile(templatePath, templateText))
		{
			// 先得到保存路径
			auto aPath = GenerateAssetPath(absolutePath, "GalGameScript", ".lua");
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
}
