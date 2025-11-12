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
#include "Texture2D.h"
#include "Video.h"
#include "../Interface/Loader.h"
#include "../Asset/TextureAsset.h"
#include <HCore/Include/Core/HSingleton.h>

namespace VisionGal {

	class VG_ENGINE_API TextureResourceManager : public Horizon::HSingletonBase<TextureResourceManager>, public VGObjectLoader
	{
	public:
		TextureResourceManager();
		TextureResourceManager(const TextureResourceManager&) = default;
		TextureResourceManager& operator=(const TextureResourceManager&) = default;
		TextureResourceManager(TextureResourceManager&&) noexcept = default;
		TextureResourceManager& operator=(TextureResourceManager&&) noexcept = default;
		~TextureResourceManager() override = default;

		static Ref<Texture2D> CreateRenderTexture(TextureAsset& asset);
		static void CreateManager();
		uint64 NumCacheTexture() const { return m_CachedTextures.size(); }
		VGObjectPtr StaticLoadObject(const String& path) override;

	private:
		std::unordered_map<String, Ref<Texture2D>> m_CachedTextures;
	};


	class VG_ENGINE_API VideoResourceManager : public Horizon::HSingletonBase<VideoResourceManager>, public VGObjectLoader
	{
	public:
		VideoResourceManager();
		VideoResourceManager(const VideoResourceManager&) = default;
		VideoResourceManager& operator=(const VideoResourceManager&) = default;
		VideoResourceManager(VideoResourceManager&&) noexcept = default;
		VideoResourceManager& operator=(VideoResourceManager&&) noexcept = default;
		~VideoResourceManager() override = default;

		static void CreateManager();
		//uint64 NumCacheTexture() const { return m_CachedVideos.size(); }
		VGObjectPtr StaticLoadObject(const String& path) override;

	private:
		//std::unordered_map<String, Ref<VideoClip>> m_CachedVideos;
	};

	class VG_ENGINE_API AudioResourceManager : public Horizon::HSingletonBase<AudioResourceManager>, public VGObjectLoader
	{
	public:
		AudioResourceManager();
		AudioResourceManager(const AudioResourceManager&) = default;
		AudioResourceManager& operator=(const AudioResourceManager&) = default;
		AudioResourceManager(AudioResourceManager&&) noexcept = default;
		AudioResourceManager& operator=(AudioResourceManager&&) noexcept = default;
		~AudioResourceManager() override = default;

		static void CreateManager();
		VGObjectPtr StaticLoadObject(const String& path) override;
	};

	VG_ENGINE_API void CreateResourceManagers();
}