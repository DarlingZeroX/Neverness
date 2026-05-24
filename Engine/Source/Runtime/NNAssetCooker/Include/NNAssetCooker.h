#pragma once

/**
 * @file NNAssetCooker.h
 * @brief 资产编译/打包编排器。
 *
 * 编排完整的构建流程：
 *   1. 读取构建清单
 *   2. 按分组收集资产
 *   3. 使用 NNPackBuilder 生成 .nnpack
 *   4. 输出构建报告
 */

#include <cstdint>
#include <functional>
#include <string>

#include "NNCookManifest.h"
#include "NNPackBuilder.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 构建结果。
 */
struct NNCookResult
{
	bool        success{false};
	std::uint32_t totalAssets{0};
	std::uint32_t cookedAssets{0};
	std::uint32_t failedAssets{0};
	std::uint32_t generatedPacks{0};
	std::string errorMessage;

	double      elapsedSeconds{0.0};
};

/**
 * @brief 构建进度回调。
 */
using NNCookProgressCallback = std::function<void(
	std::uint32_t current,
	std::uint32_t total,
	const char* phase
)>;

/**
 * @brief 资产编译/打包编排器。
 */
class NNAssetCooker
{
public:
	/** @brief 执行构建。 */
	NNCookResult Cook(const NNCookManifest& manifest,
	                  NNCookProgressCallback progress = nullptr);

	/** @brief 单个分组打包。 */
	bool CookGroup(const NNCookGroup& group,
	               const NNCookManifest& manifest,
	               NNCookProgressCallback progress = nullptr,
	               std::uint32_t groupIndex = 0,
	               std::uint32_t totalGroups = 1);
};

} // namespace NN::Runtime::Asset
