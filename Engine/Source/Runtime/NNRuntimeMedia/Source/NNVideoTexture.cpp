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
 * 已移除：
 * - NNRenderAssetManager 依赖（随 NNRenderAssets 移至 Legacy）
 *
 * 现在所有方法为空实现/stub，等待新的 GPU 纹理管理方案。
 */

#include "NNVideoTexture.h"

namespace NN::Runtime::Media
{
	NNVideoTexture::~NNVideoTexture()
	{
		Release();
	}

	bool NNVideoTexture::Initialize(std::uint32_t width, std::uint32_t height)
	{
		// 已移除：NNRenderAssetManager::CreateTextureFromPixels（NNRenderAssets 移至 Legacy）
		(void)width;
		(void)height;
		return false;
	}

	void NNVideoTexture::UpdateFrame(const std::uint8_t* rgbaPixels, std::uint32_t width, std::uint32_t height)
	{
		// 已移除：NNRenderAssetManager::UpdateTexturePixels（NNRenderAssets 移至 Legacy）
		(void)rgbaPixels;
		(void)width;
		(void)height;
	}

	std::uint64_t NNVideoTexture::GetImGuiHandle() const
	{
		return 0;
	}

	std::uint32_t NNVideoTexture::GetGLTextureId() const
	{
		return 0;
	}

	void NNVideoTexture::Release()
	{
		m_CacheKey = 0;
		m_Width = 0;
		m_Height = 0;
	}

} // namespace NN::Runtime::Media
