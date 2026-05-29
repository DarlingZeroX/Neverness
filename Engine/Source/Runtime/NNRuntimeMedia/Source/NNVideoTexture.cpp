/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

/**
 * @file NNVideoTexture.cpp
 * @brief 视频纹理实现：弥合 FVideoPlayer → GPU 的断层。
 *
 * 核心逻辑：
 * - Initialize() 通过 NNRenderAssetManager::CreateTextureFromPixels() 创建 GPU 纹理
 * - UpdateFrame() 通过 NNRenderAssetManager::UpdateTexturePixels() 热更新像素
 * - Release() 通过 NNRenderAssetManager::ReleaseTexture() 释放
 */

#include "NNVideoTexture.h"
#include "NNRuntimeRenderAssets/Include/NNRenderAssetManager.h"

namespace NN::Runtime::Media
{
	NNVideoTexture::~NNVideoTexture()
	{
		Release();
	}

	bool NNVideoTexture::Initialize(std::uint32_t width, std::uint32_t height)
	{
		if (width == 0 || height == 0)
			return false;

		m_Width = width;
		m_Height = height;

		// 创建黑色占位纹理
		std::uint8_t blackPixel[4] = {0, 0, 0, 255};
		m_CacheKey = Render::NNRenderAssetManager::Get().CreateTextureFromPixels(
			1, 1, blackPixel, 4, false);

		return m_CacheKey != 0;
	}

	void NNVideoTexture::UpdateFrame(const std::uint8_t* rgbaPixels, std::uint32_t width, std::uint32_t height)
	{
		if (!rgbaPixels || width == 0 || height == 0)
			return;

		if (width != m_Width || height != m_Height)
		{
			// 分辨率变化：重建纹理
			Release();
			if (!Initialize(width, height))
				return;
		}

		// ★ 关键调用：热更新 GPU 纹理像素 ★
		// 这填补了 FVideoPlayer::GetFrameData() → GPU 之间的断层
		std::size_t pixelSize = static_cast<std::size_t>(width) * height * 4;
		Render::NNRenderAssetManager::Get().UpdateTexturePixels(
			m_CacheKey, rgbaPixels, pixelSize);
	}

	std::uint64_t NNVideoTexture::GetImGuiHandle() const
	{
		if (m_CacheKey == 0)
			return 0;
		return Render::NNRenderAssetManager::Get().GetImGuiTextureHandle(m_CacheKey);
	}

	std::uint32_t NNVideoTexture::GetGLTextureId() const
	{
		if (m_CacheKey == 0)
			return 0;
		return static_cast<std::uint32_t>(
			Render::NNRenderAssetManager::Get().GetGLTextureId(m_CacheKey));
	}

	void NNVideoTexture::Release()
	{
		if (m_CacheKey != 0)
		{
			Render::NNRenderAssetManager::Get().ReleaseTexture(m_CacheKey);
			m_CacheKey = 0;
		}
		m_Width = 0;
		m_Height = 0;
	}

} // namespace NN::Runtime::Media
