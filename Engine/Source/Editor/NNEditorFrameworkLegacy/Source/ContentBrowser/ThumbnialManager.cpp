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

#include "ContentBrowser/ThumbnialManager.h"
#include "NNEditorFrameworkLegacy/Include/EditorCore/EditorCore.h"
#include <NNRuntimeCore/Interface/Loader.h>
#include <NNEngineLegacy/Include/Engine/ResourceManager.h>

#include "NNRuntimeAsset/Include/GalGameAsset.h"


namespace NN::Editor {

	TextureThumbnailManager::~TextureThumbnailManager()
	{
	}

	std::string& TextureThumbnailManager::GetAssetType()
	{
		static std::string type = "Texture";
		return type;
	}

	void TextureThumbnailManager::Initialize()
	{
		m_DefaultThumbnail = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/image.png");
	}

	void TextureThumbnailManager::OnUpdate()
	{
		//H_LOG_INFO("进入锁3");
		// 放在更新时间点加载略缩图，因为OpenGL上下文只能在主线程使用
		m_ReadWriteMutex.lock();

		for (auto& [path, asset] : m_CachedTextureAssets)
		{
			m_CachedTextures[path] = Runtime::TextureResourceManager::CreateRenderTexture(*asset.get());
			// 真正完成缩略图加载才移除正在加载标记
			m_LoadingTexturePaths.erase(path);
		}
		m_CachedTextureAssets.clear();

		m_ReadWriteMutex.unlock();
		//H_LOG_INFO("离开锁3");
	}

	void* TextureThumbnailManager::GetAssetThumbnail(ContentBrowserItem& item)
	{
		static Runtime::TextureAssetLoader s_TexLoader;
		auto path = item.Path;

		//H_LOG_INFO("进入锁1");
		m_ReadWriteMutex.lock();
		// 检查缓存
		{
			auto it = m_CachedTextures.find(item.Path);
			if (it != m_CachedTextures.end())
			{
				m_ReadWriteMutex.unlock();
				//H_LOG_INFO("离开锁1");
				return it->second->GetTexture()->GetShaderResourceView();
			}
		}

		// 检查正在加载略缩图
		{
			auto it = m_LoadingTexturePaths.find(item.Path);
			if (it != m_LoadingTexturePaths.end())
			{
				m_ReadWriteMutex.unlock();
				//H_LOG_INFO("离开锁1");
				return m_DefaultThumbnail->GetTexture()->GetShaderResourceView();
			}
			// 标记为正在加载
			m_LoadingTexturePaths.insert(item.Path);
		}

		// 放在这里解锁，防止后续线程死锁
		m_ReadWriteMutex.unlock();
		//H_LOG_INFO("离开锁1");

		std::thread thread([path, this]()
			{
				Ref<Runtime::VGAsset> asset;
				if (s_TexLoader.Read(path, asset))
				{
					//H_LOG_INFO("进入锁2");
					m_ReadWriteMutex.lock();
					m_CachedTextureAssets[path] = std::dynamic_pointer_cast<Runtime::TextureAsset>(asset);
					m_ReadWriteMutex.unlock();
					//H_LOG_INFO("离开锁2");
				}
			});

		thread.detach();

		// 返回默认略缩图
		return m_DefaultThumbnail->GetTexture()->GetShaderResourceView();

		// Load texture thumbnail
		//auto texture = LoadObject<Texture2D>(item.Path);
		//if (texture)
		//{
		//	return texture->GetTexture()->GetShaderResourceView();
		//}
		//
		//return m_DefaultThumbnail->GetTexture()->GetShaderResourceView();
	}

	ThumbnailManager& ThumbnailManager::GetInstance()
	{
		static ThumbnailManager instance;
		return instance;
	}

	void ThumbnailManager::Initialize()
	{
		if (m_IsInitialized == true)
			return;

		m_IsInitialized = true;

		m_DefaultThumbnails["Folder"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/folder.png");

		m_DefaultThumbnails["File"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/file.png");
		m_DefaultThumbnails["LuaScript"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/lua.png");
		m_DefaultThumbnails["Scene"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/scene.png");
		m_DefaultThumbnails["HTML"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/html.png");
		m_DefaultThumbnails["CSS"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/css.png");
		m_DefaultThumbnails["Sound"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/sound.png");
		m_DefaultThumbnails[Runtime::GLuaScriptAssetType{}.GetNameID()] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/galStoryScript.png");
		m_DefaultThumbnails["Video"] = Runtime::LoadObject<Runtime::Texture2D>(EditorCore::GetEditorResourcePathVFS() + "icons/video.png");

		m_AssetThumbnailManagers["Texture"] = MakeRef<TextureThumbnailManager>();
		for (auto& [type, manager] : m_AssetThumbnailManagers)
		{
			manager->Initialize();
		}
	}

	void ThumbnailManager::OnUpdate()
	{
		for (auto& [type, manager] : m_AssetThumbnailManagers)
		{
			manager->OnUpdate();
		}
	}

	void* ThumbnailManager::GetAssetThumbnail(ContentBrowserItem& item)
	{
		if (item.IsDirectory)
		{
			return m_DefaultThumbnails["Folder"]->GetTexture()->GetShaderResourceView();
		}

		//if (item.MetaData.AssetType == "Texture")
		//{
		//	return GetTextureThumbnail(item);  
		//}

		// 检查是否有对应的缩略图管理器
		{
			auto it = m_AssetThumbnailManagers.find(item.MetaData.AssetType);
			if (it != m_AssetThumbnailManagers.end())
			{
				return it->second->GetAssetThumbnail(item);
			}
		}

		// 使用默认缩略图
		{
			auto it = m_DefaultThumbnails.find(item.MetaData.AssetType);
			if (it != m_DefaultThumbnails.end())
			{
				return it->second->GetTexture()->GetShaderResourceView();
			}
		}

		return m_DefaultThumbnails["File"]->GetTexture()->GetShaderResourceView();
	}

	void* ThumbnailManager::GetTextureThumbnail(ContentBrowserItem& item)
	{

		return m_DefaultThumbnails["Texture"]->GetTexture()->GetShaderResourceView();
	}
}
