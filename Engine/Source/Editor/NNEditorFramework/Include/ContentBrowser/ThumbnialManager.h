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
#include "../../Config.h"
#include "NNEditorFramework/Include/EditorCore/ContentBrowser.h"
#include <unordered_map>
#include <NNEngineLegacy/Include/Render/Texture2D.h>
#include <NNRuntimeAsset/Include/TextureAsset.h>

namespace VisionGal::Editor {

	struct IThumbnailManager
	{
		virtual ~IThumbnailManager() = default;

		virtual std::string& GetAssetType() = 0;
		virtual void Initialize() = 0;
		virtual void OnUpdate() = 0;
		virtual void* GetAssetThumbnail(ContentBrowserItem& item) = 0;
	};

	struct TextureThumbnailManager : public IThumbnailManager
	{
		~TextureThumbnailManager() override;

		std::string& GetAssetType() override;
		void Initialize() override;
		void OnUpdate() override;
		void* GetAssetThumbnail(ContentBrowserItem& item) override;

	private:
		Ref<Texture2D> m_DefaultThumbnail;

		std::unordered_map<std::string, Ref<TextureAsset>> m_CachedTextureAssets;
		std::unordered_map<std::string, Ref<Texture2D>> m_CachedTextures;
		std::unordered_set<std::string> m_LoadingTexturePaths;

		std::mutex m_ReadWriteMutex;
	};

	class ThumbnailManager
	{
	public:
		static ThumbnailManager& GetInstance();

		void Initialize();

		void OnUpdate();

		void* GetAssetThumbnail(ContentBrowserItem& item);

		void* GetTextureThumbnail(ContentBrowserItem& item);
	private:
		bool m_IsInitialized = false;
		std::unordered_map<std::string, Ref<Texture2D>> m_DefaultThumbnails;

		std::unordered_map<std::string, Ref<IThumbnailManager>> m_AssetThumbnailManagers;
	};

}
