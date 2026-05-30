#pragma once

/**
 * @file IAssetResolver.h
 * @brief 资产路径解析器接口——将 GUID 解析为 VFS 路径。
 *
 * Runtime 侧定义接口，Editor / Standalone 各自实现。
 * 解耦 Runtime ↔ Editor 依赖。
 *
 * 使用方式：
 * - Editor 侧：通过 EditorAssetDatabase 解析
 * - Standalone 侧：通过 Pak/Index 文件解析
 * - 测试侧：返回硬编码路径
 */

#include <cstdint>
#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 资产路径解析器接口。
	 *
	 * 由 NNRuntimeScene 持有（非拥有），外部注入实现。
	 * NNRmlUISystem 通过此接口将 DocumentAsset GUID 解析为 VFS 路径。
	 */
	class IAssetResolver
	{
	public:
		virtual ~IAssetResolver() = default;

		/**
		 * @brief 将资产 GUID 解析为 VFS 路径。
		 * @param guid 资产 GUID（128-bit）
		 * @param outPath 输出缓冲区（UTF-8 C 字符串）
		 * @param outPathSize 缓冲区大小（含 NUL 终结符）
		 * @return true = 解析成功，outPath 已写入
		 */
		virtual bool Resolve(NNGuid guid, char* outPath, std::uint32_t outPathSize) = 0;
	};

} // namespace NN::Runtime::Scene
