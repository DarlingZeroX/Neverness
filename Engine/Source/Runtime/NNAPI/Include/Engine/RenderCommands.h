#pragma once

/**
 * @file RenderCommands.h
 * @brief 渲染命令缓冲区结构体定义——C# Scene → Flat Buffer → C++ Renderer。
 *
 * 设计原则：
 * - 所有结构体 POD（Plain Old Data），可直接 memcpy
 * - ABI 稳定：字段顺序和大小不可变
 * - 命令头 type + size 变长：向前兼容，旧客户端跳过未知命令
 * - textureHandle = Diligent ITextureView* 编码为 uint64_t（0 = 白色默认纹理）
 *
 * 命令体系：
 * - SetCamera (0x01)：设置相机矩阵和视口尺寸
 * - SetRenderPassState (0x02)：设置渲染 Pass 状态（清屏、深度等）
 * - DrawSpriteBatch (0x10)：批量精灵绘制
 * - DrawMesh (0x20)：网格绘制（第二阶段，待实现）
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════
 *  命令类型枚举
 * ═══════════════════════════════════════════ */

typedef enum NNRenderCommandType
{
    /** 设置相机（View/Projection 矩阵、视口尺寸、近远平面）。 */
    NN_RENDER_COMMAND_SET_CAMERA            = 0x01,
    /** 设置渲染 Pass 状态（清屏颜色、深度测试等标志位）。 */
    NN_RENDER_COMMAND_SET_RENDER_PASS_STATE = 0x02,
    /** 批量精灵绘制。 */
    NN_RENDER_COMMAND_DRAW_SPRITE_BATCH     = 0x10,
    /** 设置 RmlUI 文档列表（Overlay Pass 数据源）。 */
    NN_RENDER_COMMAND_SET_RML_DOCUMENTS     = 0x20,
    /* 第二阶段：
    NN_RENDER_COMMAND_DRAW_MESH             = 0x30,  // 网格绘制
    */
} NNRenderCommandType;

/* ═══════════════════════════════════════════
 *  Buffer 头部
 * ═══════════════════════════════════════════ */

/** 魔数："RNDC" (0x524E4443)。 */
#define NN_RENDER_COMMAND_BUFFER_MAGIC 0x524E4443u

/**
 * @brief 命令缓冲区头部（16 bytes）。
 *
 * 内存布局：
 * [Header][CommandHeader0][Data0][CommandHeader1][Data1]...
 */
typedef struct NNRenderCommandBufferHeader
{
    std::uint32_t magic;         /**< 必须为 NN_RENDER_COMMAND_BUFFER_MAGIC */
    std::uint32_t commandCount;  /**< 命令数量 */
    std::uint32_t totalBytes;    /**< 缓冲区总字节数（含 Header） */
    std::uint32_t reserved;      /**< 预留，必须为 0 */
} NNRenderCommandBufferHeader;

/**
 * @brief 单条命令头部（8 bytes）。
 *
 * 紧跟 data[size - 8] 字节的命令数据。
 */
typedef struct NNRenderCommandHeader
{
    std::uint32_t type;  /**< NNRenderCommandType */
    std::uint32_t size;  /**< 本条命令总字节数（含此 header + data） */
} NNRenderCommandHeader;

/* ═══════════════════════════════════════════
 *  SetCamera（type = 0x01）
 * ═══════════════════════════════════════════ */

/** SetCamera 命令总大小（header 8 + data 152 = 160 bytes）。 */
#define NN_SET_CAMERA_TOTAL_SIZE 160u

/**
 * @brief 相机数据（152 bytes）。
 *
 * 矩阵为 4x4 列主序（与 GLM / Diligent 对齐）。
 */
typedef struct NNSetCameraData
{
    float viewMatrix[16];        /**< View 矩阵（64 bytes） */
    float projectionMatrix[16];  /**< Projection 矩阵（64 bytes） */
    float viewportWidth;         /**< 视口宽度（4 bytes） */
    float viewportHeight;        /**< 视口高度（4 bytes） */
    float nearPlane;             /**< 近平面（4 bytes） */
    float farPlane;              /**< 远平面（4 bytes） */
    float orthoWidth;            /**< 正交投影宽度（4 bytes） */
    float orthoHeight;           /**< 正交投影高度（4 bytes） */
} NNSetCameraData;

/* ═══════════════════════════════════════════
 *  SetRenderPassState（type = 0x02）
 * ═══════════════════════════════════════════ */

/** SetRenderPassState 命令总大小（header 8 + data 32 = 40 bytes）。 */
#define NN_SET_RENDER_PASS_STATE_TOTAL_SIZE 40u

/** flags 位标志：启用清屏。 */
#define NN_RENDER_PASS_FLAG_CLEAR_COLOR   0x01u
/** flags 位标志：启用深度测试。 */
#define NN_RENDER_PASS_FLAG_DEPTH_TEST    0x02u
/** flags 位标志：启用深度写入。 */
#define NN_RENDER_PASS_FLAG_DEPTH_WRITE   0x04u
/* 预留：Blend、Stencil 等后续扩展 */

/**
 * @brief 渲染 Pass 状态数据（32 bytes）。
 */
typedef struct NNRenderPassStateData
{
    float clearColor[4];     /**< RGBA 清屏颜色 [0,1]（16 bytes） */
    std::uint32_t flags;     /**< 位标志（4 bytes） */
    std::uint32_t stencilRef; /**< Stencil 参考值——预留（4 bytes） */
    std::uint32_t reserved0; /**< 预留（4 bytes） */
    std::uint32_t reserved1; /**< 预留（4 bytes） */
} NNRenderPassStateData;

/* ═══════════════════════════════════════════
 *  DrawSpriteBatch（type = 0x10）
 * ═══════════════════════════════════════════ */

/** DrawSpriteBatch 命令头部大小（header 8 + data头部 16 = 24 bytes）。 */
#define NN_DRAW_SPRITE_BATCH_HEADER_SIZE 24u

/** 单个 Sprite 实例大小（120 bytes）。 */
#define NN_SPRITE_INSTANCE_SIZE 120u

/** 混合模式枚举。 */
typedef enum NNSpriteBlendMode
{
    NN_SPRITE_BLEND_ALPHA       = 0,  /**< Alpha 混合 */
    NN_SPRITE_BLEND_ADDITIVE    = 1,  /**< 叠加 */
    NN_SPRITE_BLEND_MULTIPLY    = 2,  /**< 正片叠底 */
    NN_SPRITE_BLEND_OPAQUE      = 3,  /**< 不透明 */
    NN_SPRITE_BLEND_PREMULTIPLIED = 4 /**< 预乘 Alpha */
} NNSpriteBlendMode;

/** Sprite flags 位标志：水平翻转。 */
#define NN_SPRITE_FLAG_FLIP_X  0x01u
/** Sprite flags 位标志：垂直翻转。 */
#define NN_SPRITE_FLAG_FLIP_Y  0x02u

/**
 * @brief 单个精灵实例数据（120 bytes）。
 *
 * textureHandle = Diligent ITextureView* 编码为 uint64_t。
 * 0 表示白色默认纹理。
 */
typedef struct NNSpriteInstance
{
    float transform[16];         /**< WorldMatrix（4x4 列主序，64 bytes） */
    std::uint64_t textureHandle; /**< Diligent ITextureView* 编码（8 bytes） */
    float color[4];              /**< RGBA tint [0,1]（16 bytes） */
    float uvRect[4];             /**< [u0, v0, u1, v1] Atlas UV 区域（16 bytes） */
    std::uint32_t layer;         /**< 渲染层——排序用（4 bytes） */
    std::uint32_t sortOrder;     /**< 层内排序——大的后渲染（4 bytes） */
    std::uint32_t blendMode;     /**< NNSpriteBlendMode（4 bytes） */
    std::uint32_t flags;         /**< NN_SPRITE_FLAG_* 位标志（4 bytes） */
} NNSpriteInstance;

/**
 * @brief 批量精灵绘制命令数据。
 *
 * 紧跟 spriteCount 个 NNSpriteInstance。
 * 命令总大小 = NN_DRAW_SPRITE_BATCH_HEADER_SIZE + spriteCount * NN_SPRITE_INSTANCE_SIZE
 */
typedef struct NNDrawSpriteBatchData
{
    std::uint32_t spriteCount;   /**< 精灵数量（4 bytes） */
    std::uint32_t reserved0;     /**< 预留（4 bytes） */
    std::uint32_t reserved1;     /**< 预留（4 bytes） */
    std::uint32_t reserved2;     /**< 预留（4 bytes） */
    /* 后跟 NNSpriteInstance sprites[spriteCount]（变长） */
} NNDrawSpriteBatchData;

/* ═══════════════════════════════════════════
 *  便利宏：计算命令大小
 * ═══════════════════════════════════════════ */

/** 计算 DrawSpriteBatch 命令的总字节数。 */
#define NN_DRAW_SPRITE_BATCH_TOTAL_SIZE(spriteCount) \
    (NN_DRAW_SPRITE_BATCH_HEADER_SIZE + (spriteCount) * NN_SPRITE_INSTANCE_SIZE)

/* ═══════════════════════════════════════════
 *  SetRmlDocuments（type = 0x20）
 * ═══════════════════════════════════════════ */

/** 单个 RmlDocument 条目大小（272 bytes = 256 path + 4×4 fields）。 */
#define NN_RML_DOCUMENT_ENTRY_SIZE 272u

/** SetRmlDocuments 命令头部大小（header 8 + data头部 16 = 24 bytes）。 */
#define NN_SET_RML_DOCUMENTS_HEADER_SIZE 24u

/** assetPath 缓冲区大小（含 NUL 终结符）。 */
#define NN_RML_DOCUMENT_PATH_SIZE 256u

/**
 * @brief 单个 RmlUI 文档条目（272 bytes）。
 *
 * C# 端从 Friflo ECS 收集 RmlUIDocument 组件，解析 GUID → VFS 路径后传入。
 * C++ 端直接用 assetPath 加载文档，不经过 IAssetResolver。
 */
typedef struct NNRmlDocumentEntry
{
    char     assetPath[256];     /**< VFS 路径（UTF-8，NUL 终结，最长 255 字符） */
    std::int32_t  sortOrder;    /**< 渲染排序——小的在前（4 bytes） */
    std::uint32_t viewTarget;   /**< NNRmlUIViewTarget: 0=Scene, 1=Game, 2=Both（4 bytes） */
    std::uint32_t entityHandle; /**< C# entity ID——用于 Renderer 内部 Diff（4 bytes） */
    std::uint32_t viewportId;   /**< 预留：视口 ID（0=默认，4 bytes） */
} NNRmlDocumentEntry;

/**
 * @brief SetRmlDocuments 命令数据。
 *
 * 紧跟 documentCount 个 NNRmlDocumentEntry。
 * 命令总大小 = NN_SET_RML_DOCUMENTS_HEADER_SIZE + documentCount * NN_RML_DOCUMENT_ENTRY_SIZE
 */
typedef struct NNRmlDocumentsData
{
    std::uint32_t documentCount; /**< 文档数量（4 bytes） */
    std::uint32_t reserved0;     /**< 预留（4 bytes） */
    std::uint32_t reserved1;     /**< 预留（4 bytes） */
    std::uint32_t reserved2;     /**< 预留（4 bytes） */
    /* 后跟 NNRmlDocumentEntry entries[documentCount]（变长） */
} NNRmlDocumentsData;

/** 计算 SetRmlDocuments 命令的总字节数。 */
#define NN_SET_RML_DOCUMENTS_TOTAL_SIZE(docCount) \
    (NN_SET_RML_DOCUMENTS_HEADER_SIZE + (docCount) * NN_RML_DOCUMENT_ENTRY_SIZE)

#ifdef __cplusplus
} /* extern "C" */
#endif
