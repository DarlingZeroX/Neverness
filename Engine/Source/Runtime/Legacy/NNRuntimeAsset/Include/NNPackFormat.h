#pragma once

/**
 * @file NNPackFormat.h
 * @brief .nnpack 资产包二进制格式定义。
 *
 * .nnpack 是 Neverness Engine 的资产分发格式，包含一组编译后的 .nnasset。
 * 用于运行时分发（本地包/热更新/Addressable）。
 *
 * 布局：
 *   [NNPackHeader: 64 bytes]
 *   [AssetTable: assetCount × NNPackAssetEntry]
 *   [Manifest: 变长，包含包名、标签索引、地址索引]
 *   [Asset Data: 连续 .nnasset 数据]
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 常量 ======================== */

#define NN_PACK_MAGIC        0x4E4E504Bu  /* 'NNPK' */
#define NN_PACK_VERSION      1u
#define NN_PACK_HEADER_SIZE  64
#define NN_PACK_ALIGNMENT    64

/* ======================== 包头 ======================== */

/**
 * @brief .nnpack 文件头（固定 64 字节）。
 */
#pragma pack(push, 8)
typedef struct NNPackHeader
{
    std::uint32_t magic;            /* 'NNPK' */
    std::uint32_t version;          /* 格式版本 */
    std::uint32_t assetCount;       /* 资产数量 */
    std::uint32_t flags;            /* 标志位 */

    std::uint64_t tableOffset;      /* AssetTable 偏移 */
    std::uint64_t tableSize;        /* AssetTable 大小 */

    std::uint64_t manifestOffset;   /* Manifest 偏移 */
    std::uint64_t manifestSize;     /* Manifest 大小 */

    std::uint64_t dataOffset;       /* 资产数据起始偏移 */
    std::uint64_t totalDataSize;    /* 资产数据总大小 */

    std::uint8_t  _reserved[8];     /* 填充至 64 字节 */
} NNPackHeader;
#pragma pack(pop)

/* ======================== 资产表条目 ======================== */

/**
 * @brief 包内单个资产的索引条目。
 */
#pragma pack(push, 8)
typedef struct NNPackAssetEntry
{
    std::uint64_t guidHigh;         /* 资产 GUID (high) */
    std::uint64_t guidLow;          /* 资产 GUID (low) */
    std::uint64_t typeId;           /* 资产类型 ID */
    std::uint64_t offset;           /* 在包数据区中的偏移 */
    std::uint64_t size;             /* 原始大小 */
    std::uint64_t compressedSize;   /* 压缩大小（0 = 未压缩） */
    std::uint32_t flags;            /* 条目标志 */
    std::uint32_t _pad;
} NNPackAssetEntry;
#pragma pack(pop)

/* ======================== 包标志 ======================== */

#define NN_PACK_FLAG_COMPRESSED     0x0001  /* 包数据已压缩 */
#define NN_PACK_FLAG_ENCRYPTED      0x0002  /* 包数据已加密 */
#define NN_PACK_FLAG_STREAMING      0x0004  /* 支持流式加载 */

/* ======================== 工具函数 ======================== */

/** @brief 验证包头是否有效。 */
inline int NNPackHeaderIsValid(const NNPackHeader* header)
{
    if (!header) return 0;
    if (header->magic != NN_PACK_MAGIC) return 0;
    if (header->version != NN_PACK_VERSION) return 0;
    if (header->assetCount == 0) return 0;
    return 1;
}

/** @brief 64 字节对齐。 */
inline std::uint64_t NNPackAlign(std::uint64_t offset)
{
    return (offset + NN_PACK_ALIGNMENT - 1) & ~(static_cast<std::uint64_t>(NN_PACK_ALIGNMENT) - 1);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
