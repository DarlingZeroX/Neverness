#pragma once

/**
 * @file NNAssetFormat.h
 * @brief .nnasset 運行時二進位格式定義。
 *
 * 格式佈局：
 *   [NNAssetHeader]         — 固定 96 位元組（两行 cache line）
 *   [Dependency GUIDs]      — NNGuid × dependencyCount
 *   [Blob Descriptors]      — NNBlobDescriptor × blobCount
 *   [Padding]               — 64 位元組對齊
 *   [Binary Payload]        — blob 資料連續儲存
 *
 * 設計原則：
 *   - Header 固定大小，一次 IO 可讀完
 *   - Payload 連續儲存，一次 IO 可讀完
 *   - 64 位元組對齊，cache-friendly
 */

#include <cstdint>

#include "NNNativeEngineAPI/Include/EngineTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 常數 ======================== */

/** @brief .nnasset 檔案魔數：'NNAS' */
#define NN_ASSET_MAGIC 0x4E4E4153u

/** @brief 當前格式版本 */
#define NN_ASSET_VERSION 1u

/** @brief Header 大小（位元組，两行 cache line） */
#define NN_ASSET_HEADER_SIZE 96u

/** @brief 對齊粒度 */
#define NN_ASSET_ALIGNMENT 64u

/* ======================== 標誌位 ======================== */

/** @brief 整體壓縮（payload 使用 Zstd/LZ4） */
#define NN_ASSET_FLAG_COMPRESSED     (1u << 0)

/** @brief 支援 streaming（mip/LOD 分 blob） */
#define NN_ASSET_FLAG_STREAMING      (1u << 1)

/** @brief 屬於 bundle/package 成員 */
#define NN_ASSET_FLAG_BUNDLE_MEMBER  (1u << 2)

/** @brief 包含額外型別資訊（blob 之後、payload 之前） */
#define NN_ASSET_FLAG_HAS_TYPE_INFO  (1u << 3)

/* ======================== 型別 ID（FNV-1a 預定義） ======================== */

/** @brief 紋理 2D */
#define NN_TYPE_ID_TEXTURE_2D   0x00000001ull
/** @brief 網格 */
#define NN_TYPE_ID_MESH         0x00000002ull
/** @brief 音訊剪輯 */
#define NN_TYPE_ID_AUDIO_CLIP   0x00000003ull
/** @brief 材質 */
#define NN_TYPE_ID_MATERIAL     0x00000004ull
/** @brief 著色器 */
#define NN_TYPE_ID_SHADER       0x00000005ull
/** @brief 場景 */
#define NN_TYPE_ID_SCENE        0x00000006ull
/** @brief Prefab */
#define NN_TYPE_ID_PREFAB       0x00000007ull
/** @brief 動畫 */
#define NN_TYPE_ID_ANIMATION    0x00000008ull
/** @brief Lua 腳本 */
#define NN_TYPE_ID_LUA_SCRIPT   0x00000009ull

/* ======================== Blob 型別 ======================== */

/** @brief 通用資料 blob */
#define NN_BLOB_TYPE_DATA        0u
/** @brief 紋理 mip level */
#define NN_BLOB_TYPE_MIP_LEVEL   1u
/** @brief 頂點緩衝區 */
#define NN_BLOB_TYPE_VERTEX_BUF  2u
/** @brief 索引緩衝區 */
#define NN_BLOB_TYPE_INDEX_BUF   3u
/** @brief 縮略圖 */
#define NN_BLOB_TYPE_THUMBNAIL   4u
/** @brief 音訊 PCM 資料 */
#define NN_BLOB_TYPE_AUDIO_PCM   5u
/** @brief 音訊 seek table */
#define NN_BLOB_TYPE_AUDIO_SEEK  6u
/** @brief 場景實體層次資料 */
#define NN_BLOB_TYPE_ENTITY_HIERARCHY 7u
/** @brief 元件資料（ECS blob） */
#define NN_BLOB_TYPE_COMPONENT_DATA   8u
/** @brief 型別特定元資料 */
#define NN_BLOB_TYPE_TYPE_INFO   9u

/* ======================== 格式結構 ======================== */

#pragma pack(push, 8)

/**
 * @brief .nnasset 檔案頭部（固定 96 位元組，两行 cache line）。
 *
 * 所有偏移量相對於檔案開頭。
 */
typedef struct NNAssetHeader
{
	std::uint32_t magic;            /* [0]  'NNAS' */
	std::uint32_t version;          /* [4]  格式版本 */

	NNGuid       assetGuid;         /* [8]  資產 GUID */

	std::uint64_t typeId;           /* [24] 型別 ID（FNV-1a of type name） */

	std::uint32_t dependencyCount;  /* [32] 依賴數量 */
	std::uint32_t blobCount;        /* [36] blob 數量 */

	std::uint64_t dependencyOffset; /* [40] 依賴表偏移 */
	std::uint64_t blobTableOffset;  /* [48] blob 表偏移 */
	std::uint64_t payloadOffset;    /* [56] 載荷偏移 */

	/* --- 以下在第二個 cache line（偏移 64）--- */
	std::uint64_t payloadSize;      /* [64] 載荷大小 */
	std::uint32_t flags;            /* [72] 標誌位 */
	std::uint32_t reserved0;        /* [76] 預留 */
	std::uint64_t reserved1;        /* [80] 預留 */
	std::uint64_t reserved2;        /* [88] 預留 */

} NNAssetHeader;

/**
 * @brief Blob 描述符。
 */
typedef struct NNBlobDescriptor
{
	std::uint64_t offset;          /* 相對於 payloadOffset 的偏移 */
	std::uint64_t size;            /* 未壓縮大小 */
	std::uint64_t compressedSize;  /* 壓縮後大小（0 = 未壓縮） */
	std::uint32_t blobType;        /* NN_BLOB_TYPE_* */
	std::uint32_t flags;           /* 預留標誌 */
} NNBlobDescriptor;

/**
 * @brief 紋理型別資訊（NN_TYPE_ID_TEXTURE_2D）。
 */
typedef struct NNTextureTypeInfo
{
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t format;          /* DXGI_FORMAT 等枚舉值 */
	std::uint32_t mipCount;
	std::uint32_t arraySize;
	std::uint32_t flags;           /* sRGB、generateMipmaps 等 */
} NNTextureTypeInfo;

/**
 * @brief 網格型別資訊（NN_TYPE_ID_MESH）。
 */
typedef struct NNMeshTypeInfo
{
	std::uint32_t vertexCount;
	std::uint32_t indexCount;
	std::uint32_t vertexStride;
	std::uint32_t indexFormat;     /* 16 或 32 */
	float         boundsMin[3];
	float         boundsMax[3];
} NNMeshTypeInfo;

/**
 * @brief 音訊型別資訊（NN_TYPE_ID_AUDIO_CLIP）。
 */
typedef struct NNAudioTypeInfo
{
	std::uint32_t sampleRate;
	std::uint32_t channels;
	std::uint64_t sampleCount;
	std::uint32_t format;          /* PCM16 / Float32 / Opus */
	std::uint32_t flags;
} NNAudioTypeInfo;

#pragma pack(pop)

/* ======================== 工具函式 ======================== */

/**
 * @brief 對齊至 NN_ASSET_ALIGNMENT 位元組邊界。
 */
inline std::uint64_t NNAssetAlign(std::uint64_t offset)
{
	return (offset + NN_ASSET_ALIGNMENT - 1) & ~(static_cast<std::uint64_t>(NN_ASSET_ALIGNMENT) - 1);
}

/**
 * @brief 驗證 Header 魔數與版本。
 */
inline int NNAssetHeaderIsValid(const NNAssetHeader* header)
{
	if (header == nullptr) return 0;
	if (header->magic != NN_ASSET_MAGIC) return 0;
	if (header->version != NN_ASSET_VERSION) return 0;
	return 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
