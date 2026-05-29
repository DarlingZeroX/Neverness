#pragma once

/**
 * @file NNVideoTexture.h
 * @brief 视频纹理：弥合 FVideoPlayer → GPU 的断层。
 *
 * 核心职责：
 * - 将 FVideoPlayer::GetFrameData() 的 RGBA 像素上传到 GPU
 * - 通过 NNRenderAssetManager 管理纹理生命周期
 * - 提供 ImGui handle 用于 Editor 预览
 * - 提供 GL texture ID 用于 Renderer2D 渲染
 *
 * 这是当前代码库中缺失的关键组件。
 */

#include <cstdint>
#include "../NNRuntimeMediaExport.h"

namespace NN::Runtime::Media
{
	/// 视频纹理：管理从解码帧到 GPU 纹理的桥接
	class NN_RUNTIME_MEDIA_API NNVideoTexture
	{
	public:
		NNVideoTexture() = default;
		~NNVideoTexture();

		NNVideoTexture(const NNVideoTexture&) = delete;
		NNVideoTexture& operator=(const NNVideoTexture&) = delete;

		/// 创建 GPU 纹理（指定初始尺寸）
		bool Initialize(std::uint32_t width, std::uint32_t height);

		/// 更新帧数据（每帧调用，从解码线程传入 RGBA 像素）
		void UpdateFrame(const std::uint8_t* rgbaPixels, std::uint32_t width, std::uint32_t height);

		/// 查询
		std::uint64_t GetCacheKey() const { return m_CacheKey; }
		std::uint64_t GetImGuiHandle() const;
		std::uint32_t GetGLTextureId() const;
		std::uint32_t GetWidth() const { return m_Width; }
		std::uint32_t GetHeight() const { return m_Height; }
		bool IsValid() const { return m_CacheKey != 0; }

		/// 释放 GPU 资源
		void Release();

	private:
		std::uint64_t m_CacheKey = 0;
		std::uint32_t m_Width = 0;
		std::uint32_t m_Height = 0;
	};

} // namespace NN::Runtime::Media
