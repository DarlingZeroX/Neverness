#pragma once

/**
 * @file NNPackManager.h
 * @brief .nnpack 资产包管理器。
 *
 * 职责：
 *   - 挂载/卸载 .nnpack 包
 *   - GUID → 包内资产查询
 *   - 从包中读取 .nnasset 数据
 *   - Manifest 管理（标签索引、地址索引）
 *
 * 设计：
 *   - 支持多个包同时挂载
 *   - Thread-safe
 *   - 与 NNAssetManager 集成（加载时优先查包）
 */

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNPackFormat.h"
#include "Engine/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 已挂载包的内存映射。
 */
struct NNPackMount
{
    std::string                           path;
    NNPackHeader                          header{};
    std::vector<NNPackAssetEntry>         assetTable;
    std::vector<std::uint8_t>            manifestData;
    std::vector<std::uint8_t>            fileData;    /* 整个包文件的内存映射 */

    /* GUID → assetTable 索引 */
    std::unordered_map<std::uint64_t, std::uint32_t> guidToIndex;
};

/**
 * @brief 资产包管理器。
 */
class NNPackManager
{
public:
    static NNPackManager& Instance() noexcept;

    /** @brief 挂载 .nnpack 包。 */
    bool MountPackage(const std::string& packPath);

    /** @brief 卸载 .nnpack 包。 */
    void UnmountPackage(const std::string& packPath);

    /** @brief 查询资产是否在已挂载包中。 */
    bool IsAssetInPackage(NNGuid guid) const;

    /** @brief 查询资产所在的包名。 */
    std::string GetPackageForAsset(NNGuid guid) const;

    /** @brief 从包中读取资产数据。 */
    bool ReadAssetFromPackage(NNGuid guid, std::vector<std::uint8_t>& outData) const;

    /** @brief 取得资产的 typeId（从包中查询）。 */
    std::uint64_t GetAssetTypeIdInPackage(NNGuid guid) const;

    /** @brief 列出已挂载的包。 */
    std::vector<std::string> GetMountedPackages() const;

    /** @brief 取得已挂载包数量。 */
    std::uint32_t GetMountedPackageCount() const;

    /** @brief 卸载所有包。 */
    void UnmountAll();

private:
    NNPackManager() = default;

    mutable std::mutex mutex_;
    std::vector<std::unique_ptr<NNPackMount>> mounts_;

    /* GUID.low → mount 索引 + assetTable 索引 */
    struct AssetLocation
    {
        std::uint32_t mountIndex;
        std::uint32_t entryIndex;
    };
    std::unordered_map<std::uint64_t, AssetLocation> globalIndex_;
};

} // namespace NN::Runtime::Asset
