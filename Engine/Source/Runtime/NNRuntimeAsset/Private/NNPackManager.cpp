#include "NNPackManager.h"

#include <cstring>
#include <NNRuntimeVFS/Include/VFSService.h>

namespace NN::Runtime::Asset
{

NNPackManager& NNPackManager::Instance() noexcept
{
    static NNPackManager instance;
    return instance;
}

/* ======================== 挂载 ======================== */

bool NNPackManager::MountPackage(const std::string& packPath)
{
    std::lock_guard<std::mutex> lock(mutex_);

    /* 检查是否已挂载 */
    for (const auto& m : mounts_)
    {
        if (m->path == packPath)
            return true;
    }

    /* 通过 VFS 读取整个包文件 */
    namespace VFS = NN::Runtime::VFS;
    auto file = VFS::VFSService::GetInstance()->OpenFile(
        VFS::FileInfo(packPath), VFS::IFile::FileMode::Read);
    if (!file || !file->IsOpened())
        return false;

    const auto fileSize = static_cast<std::size_t>(file->Size());
    if (fileSize < NN_PACK_HEADER_SIZE)
        return false;

    auto mount = std::make_unique<NNPackMount>();
    mount->path = packPath;
    mount->fileData.resize(fileSize);
    file->Read(mount->fileData.data(), fileSize);
    file->Close();

    /* 解析 Header */
    std::memcpy(&mount->header, mount->fileData.data(), sizeof(NNPackHeader));
    if (!NNPackHeaderIsValid(&mount->header))
        return false;

    /* 解析 AssetTable */
    const auto& hdr = mount->header;
    if (hdr.tableOffset + hdr.tableSize > fileSize)
        return false;

    const auto entryCount = hdr.assetCount;
    const auto* entries = reinterpret_cast<const NNPackAssetEntry*>(
        mount->fileData.data() + hdr.tableOffset);

    mount->assetTable.assign(entries, entries + entryCount);

    /* 构建 GUID → 索引 */
    const auto mountIdx = static_cast<std::uint32_t>(mounts_.size());
    for (std::uint32_t i = 0; i < entryCount; ++i)
    {
        const auto& entry = mount->assetTable[i];
        mount->guidToIndex[entry.guidLow] = i;

        /* 全局索引 */
        AssetLocation loc{};
        loc.mountIndex = mountIdx;
        loc.entryIndex = i;
        globalIndex_[entry.guidLow] = loc;
    }

    /* 解析 Manifest */
    if (hdr.manifestOffset > 0 && hdr.manifestOffset + hdr.manifestSize <= fileSize)
    {
        mount->manifestData.resize(hdr.manifestSize);
        std::memcpy(mount->manifestData.data(),
                     mount->fileData.data() + hdr.manifestOffset,
                     hdr.manifestSize);
    }

    mounts_.push_back(std::move(mount));
    return true;
}

/* ======================== 卸载 ======================== */

void NNPackManager::UnmountPackage(const std::string& packPath)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = mounts_.begin(); it != mounts_.end(); ++it)
    {
        if ((*it)->path == packPath)
        {
            /* 从全局索引中移除 */
            for (const auto& [guidLow, _] : (*it)->guidToIndex)
                globalIndex_.erase(guidLow);

            mounts_.erase(it);
            return;
        }
    }
}

void NNPackManager::UnmountAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    mounts_.clear();
    globalIndex_.clear();
}

/* ======================== 查询 ======================== */

bool NNPackManager::IsAssetInPackage(NNGuid guid) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return globalIndex_.find(guid.low) != globalIndex_.end();
}

std::string NNPackManager::GetPackageForAsset(NNGuid guid) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = globalIndex_.find(guid.low);
    if (it == globalIndex_.end())
        return {};

    return mounts_[it->second.mountIndex]->path;
}

bool NNPackManager::ReadAssetFromPackage(NNGuid guid, std::vector<std::uint8_t>& outData) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = globalIndex_.find(guid.low);
    if (it == globalIndex_.end())
        return false;

    const auto& mount = mounts_[it->second.mountIndex];
    const auto& entry = mount->assetTable[it->second.entryIndex];

    /* 检查数据边界 */
    const auto dataStart = mount->header.dataOffset + entry.offset;
    const auto readSize = entry.compressedSize > 0 ? entry.compressedSize : entry.size;

    if (dataStart + readSize > mount->fileData.size())
        return false;

    /* 如果未压缩，直接拷贝 */
    if (entry.compressedSize == 0)
    {
        outData.resize(entry.size);
        std::memcpy(outData.data(), mount->fileData.data() + dataStart, entry.size);
    }
    else
    {
        /* TODO: 解压缩（Zstd/LZ4）
         * 当前降级：直接拷贝压缩数据 */
        outData.resize(readSize);
        std::memcpy(outData.data(), mount->fileData.data() + dataStart, readSize);
    }

    return true;
}

std::uint64_t NNPackManager::GetAssetTypeIdInPackage(NNGuid guid) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = globalIndex_.find(guid.low);
    if (it == globalIndex_.end())
        return 0;

    return mounts_[it->second.mountIndex]->assetTable[it->second.entryIndex].typeId;
}

/* ======================== 枚举 ======================== */

std::vector<std::string> NNPackManager::GetMountedPackages() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(mounts_.size());
    for (const auto& m : mounts_)
        result.push_back(m->path);
    return result;
}

std::uint32_t NNPackManager::GetMountedPackageCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<std::uint32_t>(mounts_.size());
}

} // namespace NN::Runtime::Asset
