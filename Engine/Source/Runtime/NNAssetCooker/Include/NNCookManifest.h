#pragma once

/**
 * @file NNCookManifest.h
 * @brief 构建清单 — 描述哪些资产需要编译、打包目标。
 *
 * 构建清单由 Editor 构建管线生成，传递给 NNAssetCooker 执行。
 */

#include <cstdint>
#include <string>
#include <vector>

#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 待编译资产条目。
 */
struct NNCookAssetEntry
{
	NNGuid         guid{};              /* 资产 GUID */
	std::uint64_t  typeId{0};           /* 资产类型 ID */
	std::string    sourcePath;          /* .nnasset 源路径 */
	std::uint32_t  groupIndex{0};       /* 所属分组索引 */
	bool           includeInBuild{true}; /* 是否包含在构建中 */
};

/**
 * @brief 构建分组。
 */
struct NNCookGroup
{
	std::string    name;                /* 分组名称 */
	std::string    address;             /* Addressable 地址 */
	std::uint32_t  strategy{0};         /* 打包策略：0=PackTogether, 1=PackSeparately */
	std::string    outputPath;          /* .nnpack 输出路径 */
};

/**
 * @brief 构建清单。
 */
class NNCookManifest
{
public:
	/** @brief 添加待编译资产。 */
	void AddAsset(const NNCookAssetEntry& entry);

	/** @brief 添加构建分组。 */
	void AddGroup(const NNCookGroup& group);

	/** @brief 设置输出根目录。 */
	void SetOutputRoot(const std::string& path);

	/** @brief 设置库根目录（Library/Imported/）。 */
	void SetLibraryRoot(const std::string& path);

	/* 访问器 */
	const std::vector<NNCookAssetEntry>& GetAssets() const { return assets_; }
	const std::vector<NNCookGroup>& GetGroups() const { return groups_; }
	const std::string& GetOutputRoot() const { return outputRoot_; }
	const std::string& GetLibraryRoot() const { return libraryRoot_; }

	std::uint32_t GetAssetCount() const { return static_cast<std::uint32_t>(assets_.size()); }
	std::uint32_t GetGroupCount() const { return static_cast<std::uint32_t>(groups_.size()); }

	/** @brief 清空清单。 */
	void Clear();

private:
	std::vector<NNCookAssetEntry> assets_;
	std::vector<NNCookGroup> groups_;
	std::string outputRoot_;
	std::string libraryRoot_;
};

} // namespace NN::Runtime::Asset
