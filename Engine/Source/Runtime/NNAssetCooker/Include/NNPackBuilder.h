#pragma once

/**
 * @file NNPackBuilder.h
 * @brief .nnpack 包构建器。
 *
 * 将一组 .nnasset 文件打包为 .nnpack 格式。
 */

#include <cstdint>
#include <string>
#include <vector>

#include "NNPackFormat.h"
#include "NNCookManifest.h"

namespace NN::Runtime::Asset
{

/**
 * @brief .nnpack 构建器。
 *
 * 使用方法：
 *   NNPackBuilder builder;
 *   builder.SetOutputPath("Build/Windows/game.nnpack");
 *   builder.AddAsset(guid, typeId, nnassetData);
 *   builder.Build();
 */
class NNPackBuilder
{
public:
	/** @brief 设置输出路径。 */
	void SetOutputPath(const std::string& path);

	/** @brief 添加资产数据。 */
	void AddAsset(NNGuid guid, std::uint64_t typeId,
	              const std::vector<std::uint8_t>& data);

	/** @brief 设置包名（写入 Manifest）。 */
	void SetPackageName(const std::string& name);

	/** @brief 构建 .nnpack 文件。 */
	bool Build();

	/** @brief 获取构建的资产数量。 */
	std::uint32_t GetAssetCount() const;

	/** @brief 清空所有资产。 */
	void Clear();

private:
	struct PendingAsset
	{
		NNGuid guid{};
		std::uint64_t typeId{0};
		std::vector<std::uint8_t> data;
	};

	std::string outputPath_;
	std::string packageName_;
	std::vector<PendingAsset> assets_;
};

} // namespace NN::Runtime::Asset
