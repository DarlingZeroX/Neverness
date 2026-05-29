#pragma once

/**
 * @file NNVideoClipAsset.h
 * @brief 运行时视频资产：持有视频元信息和解码帧数据/解码器引用。
 */

#include <cstdint>
#include <vector>
#include "NNRuntimeAsset/Include/NNAssetFormat.h"
#include "NNRuntimeAsset/Include/NNAssetHandle.h"
#include "../NNRuntimeMediaExport.h"

namespace NN::Runtime::Media
{
	/// 运行时视频资产
	class NN_RUNTIME_MEDIA_API NNVideoClipAsset
	{
	public:
		/// 从 NNAssetManager handle 初始化
		bool Initialize(Asset::NNAssetHandleGeneric handle);

		/// 流式模式：创建解码器
		bool StartDecoding();
		void StopDecoding();

		/// 获取当前帧数据（RGBA）
		const std::uint8_t* GetCurrentFrameData() const;
		double GetCurrentPTS() const;

		/// 元信息
		std::uint32_t GetWidth() const;
		std::uint32_t GetHeight() const;
		double GetDuration() const;
		double GetFPS() const;

	private:
		NNVideoTypeInfo m_TypeInfo{};
		Asset::NNAssetHandleGeneric m_Handle{};

		// 非流式模式
		std::vector<std::vector<std::uint8_t>> m_DecodedFrames;
		std::uint64_t m_CurrentFrameIndex = 0;
	};

} // namespace NN::Runtime::Media
