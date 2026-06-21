#pragma once

/**
 * @file NNAudioClipAsset.h
 * @brief 运行时音频资产：持有解码后的音频数据或流式解码器引用。
 *
 * 已移除：NNRuntimeAsset 依赖（移至 Legacy）。
 * NNAssetHandleGeneric 替换为 uint64_t，NNAudioTypeInfo 改用本地定义。
 */

#include <cstdint>
#include <vector>
#include "MediaAssetTypes.h"
#include "../NNRuntimeMediaExport.h"

namespace NN::Runtime::Media
{
	/// 运行时音频资产
	class NN_RUNTIME_MEDIA_API NNAudioClipAsset
	{
	public:
		/// 从 NNAssetManager handle 初始化（handle 为 uint64_t）
		bool Initialize(std::uint64_t handle);

		/// 全量加载模式：PCM 数据已在内存中
		const std::int16_t* GetPCMData() const;
		std::uint64_t GetSampleCount() const;

		/// 元信息
		std::uint32_t GetSampleRate() const;
		std::uint32_t GetChannels() const;
		double GetDuration() const;
		bool IsStreaming() const;

	private:
		NNAudioTypeInfo m_TypeInfo{};
		std::uint64_t m_Handle = 0;
		std::vector<std::uint8_t> m_PCMData;
	};

} // namespace NN::Runtime::Media
