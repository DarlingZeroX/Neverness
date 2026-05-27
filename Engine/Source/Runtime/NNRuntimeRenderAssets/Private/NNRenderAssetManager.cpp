/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNRenderAssetManager.h"
#include "NNRuntimeRHI/Include/OpenGL/Texture2D.h"
#include "NNRuntimeRHI/Interface/Texture.h"
#include "NNRuntimeAsset/Include/NNAssetManager.h"
#include "NNCore/Interface/HLog.h"

//#include <glad/glad.h>
//#include <NNRuntimeRHI/Include/OpenGL/IncludeGladGL3.h>

namespace NN::Runtime::Render
{

// ===== OpenGL 格式映射 =====
// 将引擎级 NNTextureFormat 映射为 OpenGL GLenum

struct GLFormatMapping
{
    GLenum InternalFormat;
    GLenum Format;
    GLenum Type;
};

static GLFormatMapping MapToGLFormat(NNTextureFormat fmt)
{
    switch (fmt)
    {
    case NNTextureFormat::R8_UNorm:    return { GL_R8,    GL_RED,  GL_UNSIGNED_BYTE };
    case NNTextureFormat::RG8_UNorm:   return { GL_RG8,   GL_RG,   GL_UNSIGNED_BYTE };
    case NNTextureFormat::RGB8_UNorm:  return { GL_RGB8,  GL_RGB,  GL_UNSIGNED_BYTE };
    case NNTextureFormat::RGBA8_UNorm: return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
    default:                           return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
    }
}

// ===== NNRenderAssetManager 实现 =====

NNRenderAssetManager& NNRenderAssetManager::Get()
{
    static NNRenderAssetManager instance;
    return instance;
}

bool NNRenderAssetManager::Initialize()
{
    std::lock_guard lock(m_Mutex);
    if (m_Initialized)
        return true;

    m_EntryCache.clear();
    m_NextKey = 1;
    m_CurrentFrame = 0;
    m_Initialized = true;
    return true;
}

void NNRenderAssetManager::Shutdown()
{
    std::lock_guard lock(m_Mutex);
    // shared_ptr 自动释放所有 GPU 纹理
    m_EntryCache.clear();
    m_Initialized = false;
    m_NextKey = 1;
}

uint64_t NNRenderAssetManager::CreateTextureFromSource(const NNTextureSourceAsset& source)
{
    if (!m_Initialized || !source.IsValid())
        return 0;

    std::lock_guard lock(m_Mutex);
    auto* res = UploadTextureInternal(source);
    if (!res)
        return 0;
    return m_NextKey - 1;
}

uint64_t NNRenderAssetManager::CreateTextureFromPixels(
    uint32_t width, uint32_t height,
    const uint8_t* pixels, size_t pixelSize,
    bool isSRGB)
{
    if (!m_Initialized || !pixels || pixelSize == 0)
        return 0;

    NNTextureSourceAsset source;
    std::vector<uint8_t> pixelCopy(pixels, pixels + pixelSize);
    source.SetFromDecodedImage(width, height, NNTextureFormat::RGBA8_UNorm,
                               std::move(pixelCopy), isSRGB, true);

    std::lock_guard lock(m_Mutex);
    auto* res = UploadTextureInternal(source);
    if (!res)
        return 0;
    return m_NextKey - 1;
}

uint64_t NNRenderAssetManager::LoadTextureFromAsset(uint64_t assetHandle, uint64_t guidLow)
{
    if (!m_Initialized || assetHandle == 0)
    {
        H_LOG_WARN("[RenderAssetManager] LoadTextureFromAsset: 未初始化或 handle=0");
        return 0;
    }

    // 如果有 guidLow，先检查是否已缓存
    if (guidLow != 0)
    {
        std::lock_guard lock(m_Mutex);
        auto it = m_GuidToCacheKeyMap.find(guidLow);
        if (it != m_GuidToCacheKeyMap.end() && m_EntryCache.count(it->second))
        {
            H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: GUID.Low=%llu 已缓存 key=%llu", guidLow, it->second);
            return it->second;
        }
    }

    H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: 开始加载 handle=%llu", assetHandle);

    auto& assetMgr = NN::Runtime::Asset::NNAssetManager::Instance();

    /* 查找 TypeInfo blob → 获取纹理元数据 */
    const void* typeInfoData = nullptr;
    auto* typeInfoDesc = assetMgr.GetBlobByType(assetHandle, NN_BLOB_TYPE_TYPE_INFO, &typeInfoData);
    if (!typeInfoDesc || !typeInfoData || typeInfoDesc->size < sizeof(NNTextureTypeInfo))
    {
        H_LOG_ERROR("[RenderAssetManager] LoadTextureFromAsset: handle=%llu 缺少 TypeInfo blob", assetHandle);
        return 0;
    }

    auto* texInfo = static_cast<const NNTextureTypeInfo*>(typeInfoData);
    H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: TypeInfo desc offset=%llu size=%llu",
               typeInfoDesc->offset, typeInfoDesc->size);

    /* Hex dump 前 24 字节 */
    {
        auto* raw = static_cast<const unsigned char*>(typeInfoData);
        H_LOG_INFO("[RenderAssetManager] TypeInfo hex: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7],
            raw[8], raw[9], raw[10], raw[11], raw[12], raw[13], raw[14], raw[15],
            raw[16], raw[17], raw[18], raw[19], raw[20], raw[21], raw[22], raw[23]);
    }

    H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: TypeInfo %ux%u format=%u mipCount=%u flags=0x%x",
               texInfo->width, texInfo->height, texInfo->format, texInfo->mipCount, texInfo->flags);

    /* 合理性校验：宽高在有效范围内，format 为已知值 */
    if (texInfo->width == 0 || texInfo->height == 0 || texInfo->width > 16384 || texInfo->height > 16384)
    {
        H_LOG_ERROR("[RenderAssetManager] LoadTextureFromAsset: TypeInfo 宽高无效 %ux%u（可能 offset 指向错误数据）",
                    texInfo->width, texInfo->height);
        return 0;
    }
    if (texInfo->format > 4)
    {
        H_LOG_ERROR("[RenderAssetManager] LoadTextureFromAsset: TypeInfo format=%u 无效", texInfo->format);
        return 0;
    }

    /* 查找 DATA blob → 获取像素数据 */
    const void* pixelData = nullptr;
    auto* dataDesc = assetMgr.GetBlobByType(assetHandle, NN_BLOB_TYPE_DATA, &pixelData);
    if (!dataDesc || !pixelData || dataDesc->size == 0)
    {
        H_LOG_ERROR("[RenderAssetManager] LoadTextureFromAsset: handle=%llu 缺少 DATA blob", assetHandle);
        return 0;
    }

    H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: DATA blob desc offset=%llu size=%llu", dataDesc->offset, dataDesc->size);

    /* DATA blob hex dump 前 16 字节 */
    {
        auto* raw = static_cast<const unsigned char*>(pixelData);
        H_LOG_INFO("[RenderAssetManager] DATA hex: %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
            raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7],
            raw[8], raw[9], raw[10], raw[11], raw[12], raw[13], raw[14], raw[15]);
    }

    /* 构造 Source Asset */
    auto format = static_cast<NNTextureFormat>(texInfo->format);
    bool isSRGB = (texInfo->flags & 1) != 0;
    bool hasAlpha = (format == NNTextureFormat::RGBA8_UNorm);

    std::vector<uint8_t> pixelCopy(
        static_cast<const uint8_t*>(pixelData),
        static_cast<const uint8_t*>(pixelData) + dataDesc->size);

    NNTextureSourceAsset source;
    source.SetFromDecodedImage(texInfo->width, texInfo->height, format,
                               std::move(pixelCopy), isSRGB, hasAlpha);

    std::lock_guard lock(m_Mutex);
    auto* res = UploadTextureInternal(source);
    if (!res)
    {
        H_LOG_ERROR("[RenderAssetManager] LoadTextureFromAsset: GPU 上传失败 %ux%u",
                    source.GetWidth(), source.GetHeight());
        return 0;
    }

    uint64_t key = m_NextKey - 1;
    H_LOG_INFO("[RenderAssetManager] LoadTextureFromAsset: 成功 handle=%llu → key=%llu %ux%u",
               assetHandle, key, source.GetWidth(), source.GetHeight());

    // 注册 GUID → cache key 映射
    if (guidLow != 0)
        m_GuidToCacheKeyMap[guidLow] = key;

    return key;
}

uint64_t NNRenderAssetManager::CacheResource(std::unique_ptr<NNTextureResource> resource)
{
    if (!resource)
        return 0;

    std::lock_guard lock(m_Mutex);

    uint64_t key = m_NextKey++;

    auto entry = std::make_unique<RenderAssetCacheEntry>();
    entry->OwnedTexture = nullptr; // 已在外部创建
    entry->Resource = std::move(resource);
    entry->GPUMemory = entry->Resource->m_Desc.Width * entry->Resource->m_Desc.Height * 4; // 估算

    m_EntryCache[key] = std::move(entry);
    return key;
}

NNTextureResource* NNRenderAssetManager::GetTextureResource(uint64_t key)
{
    std::lock_guard lock(m_Mutex);

    auto it = m_EntryCache.find(key);
    if (it == m_EntryCache.end())
        return nullptr;

    auto* res = it->second->Resource.get();
    res->m_LastUsedFrame = m_CurrentFrame;
    return res;
}

void NNRenderAssetManager::ReleaseTexture(uint64_t key)
{
    std::lock_guard lock(m_Mutex);
    m_EntryCache.erase(key);
}

void NNRenderAssetManager::ReloadTexture(uint64_t key, const NNTextureSourceAsset& source)
{
    if (!source.IsValid())
        return;

    std::lock_guard lock(m_Mutex);

    auto it = m_EntryCache.find(key);
    if (it == m_EntryCache.end())
        return;

    // 上传新纹理
    auto* newTex = UploadTextureInternal(source);
    if (!newTex)
        return;

    // 更新缓存条目（旧 shared_ptr 自动释放旧 GPU 资源）
    // 注意：这里需要获取新条目的 key
    // 简化方案：直接替换条目内容
    uint64_t newKey = m_NextKey - 1; // UploadTextureInternal 刚分配的 key
    auto newIt = m_EntryCache.find(newKey);
    if (newIt != m_EntryCache.end())
    {
        // 将新条目移到旧 key 下
        it->second = std::move(newIt->second);
        m_EntryCache.erase(newKey);
    }
}

uint64_t NNRenderAssetManager::GetImGuiTextureHandle(uint64_t key)
{
    auto* res = GetTextureResource(key);
    if (!res)
        return 0;
    return res->GetImGuiHandle();
}

uint64_t NNRenderAssetManager::GetCacheKeyByGuidLow(uint64_t guidLow) const
{
    if (guidLow == 0)
        return 0;
    std::lock_guard lock(m_Mutex);
    auto it = m_GuidToCacheKeyMap.find(guidLow);
    if (it != m_GuidToCacheKeyMap.end())
        return it->second;
    return 0;
}

uint64_t NNRenderAssetManager::GetGLTextureId(uint64_t cacheKey) const
{
    if (cacheKey == 0)
        return 0;
    std::lock_guard lock(m_Mutex);
    auto it = m_EntryCache.find(cacheKey);
    if (it == m_EntryCache.end())
        return 0;
    auto* res = it->second->Resource.get();
    if (!res)
        return 0;
    return res->GetImGuiHandle();
}

void NNRenderAssetManager::EvictLRU(size_t targetMemoryBytes)
{
    std::lock_guard lock(m_Mutex);

    // 简单实现：遍历找到最旧的条目并删除
    // 直到总内存低于 target
    size_t totalMem = 0;
    for (auto& [k, e] : m_EntryCache)
        totalMem += e->GPUMemory;

    while (totalMem > targetMemoryBytes && m_EntryCache.size() > 1)
    {
        uint64_t oldestKey = 0;
        uint64_t oldestFrame = UINT64_MAX;

        for (auto& [k, e] : m_EntryCache)
        {
            if (e->Resource && e->Resource->m_LastUsedFrame < oldestFrame)
            {
                oldestFrame = e->Resource->m_LastUsedFrame;
                oldestKey = k;
            }
        }

        if (oldestKey == 0)
            break;

        auto it = m_EntryCache.find(oldestKey);
        totalMem -= it->second->GPUMemory;
        m_EntryCache.erase(it);
    }
}

void NNRenderAssetManager::SetCurrentFrame(uint64_t frame)
{
    m_CurrentFrame = frame;
}

size_t NNRenderAssetManager::GetCachedTextureCount() const
{
    std::lock_guard lock(m_Mutex);
    return m_EntryCache.size();
}

size_t NNRenderAssetManager::GetEstimatedGPUMemory() const
{
    std::lock_guard lock(m_Mutex);
    size_t total = 0;
    for (auto& [k, e] : m_EntryCache)
        total += e->GPUMemory;
    return total;
}

// ===== 内部实现 =====

NNTextureResource* NNRenderAssetManager::UploadTextureInternal(const NNTextureSourceAsset& source)
{
    H_LOG_INFO("[UploadTextureInternal] 开始 %ux%u format=%u mips=%u",
        source.GetWidth(), source.GetHeight(), (unsigned)source.GetFormat(), source.GetMipCount());

    const auto& baseMip = source.GetMip(0);
    H_LOG_INFO("[UploadTextureInternal] baseMip pixels=%zu", baseMip.Pixels.size());

    // 构造 RHI TextureDesc
    Runtime::VGFX::TextureDesc desc;
    desc.Width = static_cast<int>(source.GetWidth());
    desc.Height = static_cast<int>(source.GetHeight());

    auto glFmt = MapToGLFormat(source.GetFormat());
    desc.InternalFormat = static_cast<int>(glFmt.InternalFormat);
    desc.Format = glFmt.Format;
    desc.Type = glFmt.Type;
    desc.Data = const_cast<uint8_t*>(baseMip.Pixels.data());
    desc.DataSize = static_cast<unsigned int>(baseMip.Pixels.size());

    H_LOG_INFO("[UploadTextureInternal] GL: internalFormat=0x%x format=0x%x type=0x%x dataSize=%u",
        desc.InternalFormat, desc.Format, desc.Type, desc.DataSize);

    // 通过 RHI 创建 GPU Texture
    H_LOG_INFO("[UploadTextureInternal] 调用 CreateFromMemory...");
    auto glTexture = OpenGL::Texture2D::CreateFromMemory(desc);
    if (!glTexture)
    {
        H_LOG_ERROR("[UploadTextureInternal] CreateFromMemory 返回 null");
        return nullptr;
    }
    H_LOG_INFO("[UploadTextureInternal] CreateFromMemory 成功");

    // 创建 TextureResource（观察指针）
    auto resource = std::make_unique<NNTextureResource>();
    resource->m_Desc.Width = source.GetWidth();
    resource->m_Desc.Height = source.GetHeight();
    resource->m_Desc.MipCount = source.GetMipCount();
    resource->m_Desc.Format = source.GetFormat();
    resource->m_Desc.IsSRGB = source.IsSRGB();
    resource->m_RHITexture = glTexture.get();
    H_LOG_INFO("[UploadTextureInternal] 调用 GetShaderResourceView...");
    resource->m_RHIShaderResourceView = glTexture->GetShaderResourceView();
    resource->m_Residency = NNTextureResidency::Resident;
    resource->m_LastUsedFrame = m_CurrentFrame;

    // 估算 GPU 内存
    size_t gpuMem = static_cast<size_t>(source.GetWidth()) * source.GetHeight() * 4;

    // 分配 key
    uint64_t key = m_NextKey++;

    // 存入缓存（shared_ptr 持有所有权）
    auto entry = std::make_unique<RenderAssetCacheEntry>();
    entry->OwnedTexture = std::move(glTexture);   // shared_ptr 保持纹理存活
    entry->Resource = std::move(resource);
    entry->GPUMemory = gpuMem;

    m_EntryCache[key] = std::move(entry);

    return m_EntryCache[key]->Resource.get();
}

} // namespace NN::Runtime::Render
