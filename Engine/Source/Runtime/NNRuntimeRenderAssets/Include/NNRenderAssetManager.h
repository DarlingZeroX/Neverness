/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNTextureResource.h"
#include "NNTextureSourceAsset.h"
#include "NNTextureCache.h"
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../../NNRuntimeRenderAssets/NNRuntimeRenderAssetsExport.h"

namespace NN::Runtime:: Render
{

/// 缓存条目：通过 shared_ptr<void> 持有 GPU 纹理所有权
/// shared_ptr<void> 保留原始 shared_ptr<Texture2D> 的 deleter，无需完整类型定义
struct NN_RUNTIME_RENDER_ASSETS_API RenderAssetCacheEntry
{
    std::shared_ptr<void> OwnedTexture;   // 实际是 shared_ptr<OpenGL::Texture2D>，deleter 已捕获
    std::unique_ptr<NNTextureResource> Resource;
    size_t GPUMemory = 0;
};

/// Render Asset Manager
/// 负责 CPU Asset → GPU Resource 的生命周期管理
class NN_RUNTIME_RENDER_ASSETS_API NNRenderAssetManager
{
public:
    static NNRenderAssetManager& Get();

    /// 初始化（需要 RHI 已就绪）
    bool Initialize();
    void Shutdown();

    /// 从 Source Asset 直接创建 GPU Texture（Editor / 直接创建场景）
    /// @return 缓存 key，0 = 失败
    uint64_t CreateTextureFromSource(const NNTextureSourceAsset& source);

    /// 从原始像素数据创建 GPU Texture
    /// @return 缓存 key，0 = 失败
    uint64_t CreateTextureFromPixels(
        uint32_t width, uint32_t height,
        const uint8_t* pixels, size_t pixelSize,
        bool isSRGB = false
    );

    /// 从已加载的 .nnasset 资源句柄创建 GPU Texture
    /// 读取 blob[0]（DATA）反序列化为 NNTextureSourceAsset，再上传 GPU
    /// @param assetHandle AssetManager 返回的资源句柄
    /// @param guidLow 资产 GUID.Low（可选，用于建立 GUID→cacheKey 索引）
    /// @return 缓存 key，0 = 失败
    uint64_t LoadTextureFromAsset(uint64_t assetHandle, uint64_t guidLow = 0);

    /// 从已解析的 blob 数据直接创建 GPU Texture（避免跨模块单例问题）
    uint64_t LoadTextureFromBlob(const void* typeInfoData, uint64_t typeInfoSize,
                                 const void* pixelData, uint64_t pixelDataSize,
                                 uint64_t guidLow = 0);

    /// 注册一个 TextureResource 到缓存（返回用于查询的 key）
    uint64_t CacheResource(std::unique_ptr<NNTextureResource> resource);

    /// 通过 key 获取 GPU Texture Resource
    NNTextureResource* GetTextureResource(uint64_t key);

    /// 释放 GPU Texture 资源
    void ReleaseTexture(uint64_t key);

    /// 重载 Texture（Hot Reload 场景）
    void ReloadTexture(uint64_t key, const NNTextureSourceAsset& source);

    /// 获取 ImGui Texture Handle
    uint64_t GetImGuiTextureHandle(uint64_t key);

    /// 驱逐最近最少使用的 Texture（预留）
    void EvictLRU(size_t targetMemoryBytes);

    /// 更新帧号（用于 LRU / Residency）
    void SetCurrentFrame(uint64_t frame);

    /// 获取缓存统计
    size_t GetCachedTextureCount() const;
    size_t GetEstimatedGPUMemory() const;

    /// 通过 GUID.Low 查询已缓存的 cache key（O(1)），0 = 未找到
    uint64_t GetCacheKeyByGuidLow(uint64_t guidLow) const;

    /// 通过 cache key 获取 GL texture ID（GLuint 零扩展为 uint64_t），0 = 未找到
    uint64_t GetGLTextureId(uint64_t cacheKey) const;

    /// 热更新已存在纹理的像素数据（用于视频帧逐帧更新）
    /// @param cacheKey 已缓存的纹理 key
    /// @param pixels 新的 RGBA 像素数据
    /// @param pixelSize 像素数据大小（bytes）
    /// @return 是否更新成功
    bool UpdateTexturePixels(uint64_t cacheKey, const uint8_t* pixels, size_t pixelSize);

    bool IsInitialized() const { return m_Initialized; }

private:
    NNRenderAssetManager() = default;
    ~NNRenderAssetManager() = default;

    NNTextureResource* UploadTextureInternal(const NNTextureSourceAsset& source);

    mutable std::mutex m_Mutex;
    std::unordered_map<uint64_t, std::unique_ptr<RenderAssetCacheEntry>> m_EntryCache;
    std::unordered_map<uint64_t, uint64_t> m_GuidToCacheKeyMap;  // GUID.Low → cache key
    uint64_t m_CurrentFrame = 0;
    uint64_t m_NextKey = 1;  // 自增 key 分配器
    bool m_Initialized = false;
};

} // namespace NN::Runtime::Render
