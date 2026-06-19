using System.Runtime.InteropServices;

namespace Neverness.Rendering.Diligent.Commands;

/// <summary>
/// 渲染命令类型枚举——与 C++ NNRenderCommandType 对齐。
/// </summary>
public enum RenderCommandType : uint
{
    /// <summary>设置相机（View/Projection 矩阵、视口尺寸、近远平面）。</summary>
    SetCamera = 0x01,
    /// <summary>设置渲染 Pass 状态（清屏颜色、深度测试等标志位）。</summary>
    SetRenderPassState = 0x02,
    /// <summary>批量精灵绘制。</summary>
    DrawSpriteBatch = 0x10,
    /// <summary>设置 RmlUI 文档列表（Overlay Pass 数据源）。</summary>
    SetRmlDocuments = 0x20,
}

/// <summary>
/// 命令缓冲区头部（16 bytes）——与 C++ NNRenderCommandBufferHeader 逐字节对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct RenderCommandBufferHeader
{
    /// <summary>魔数，必须为 0x524E4443 ("RNDC")。</summary>
    public uint Magic;
    /// <summary>命令数量。</summary>
    public uint CommandCount;
    /// <summary>缓冲区总字节数（含 Header）。</summary>
    public uint TotalBytes;
    /// <summary>预留，必须为 0。</summary>
    public uint Reserved;
}

/// <summary>
/// 单条命令头部（8 bytes）——与 C++ NNRenderCommandHeader 逐字节对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct RenderCommandHeader
{
    /// <summary>命令类型（RenderCommandType）。</summary>
    public uint Type;
    /// <summary>本条命令总字节数（含此 header + data）。</summary>
    public uint Size;
}

/// <summary>
/// SetCamera 命令数据（144 bytes）——与 C++ NNSetCameraData 逐字节对齐。
/// 矩阵为 4x4 列主序（与 GLM / Diligent 对齐）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public unsafe struct SetCameraData
{
    /// <summary>View 矩阵（4x4 列主序，64 bytes）。</summary>
    public fixed float ViewMatrix[16];
    /// <summary>Projection 矩阵（4x4 列主序，64 bytes）。</summary>
    public fixed float ProjectionMatrix[16];
    /// <summary>视口宽度。</summary>
    public float ViewportWidth;
    /// <summary>视口高度。</summary>
    public float ViewportHeight;
    /// <summary>近平面。</summary>
    public float NearPlane;
    /// <summary>远平面。</summary>
    public float FarPlane;
    /// <summary>正交投影宽度。</summary>
    public float OrthoWidth;
    /// <summary>正交投影高度。</summary>
    public float OrthoHeight;
}

/// <summary>
/// 渲染 Pass 状态标志位——与 C++ NN_RENDER_PASS_FLAG_* 对齐。
/// </summary>
[Flags]
public enum RenderPassFlags : uint
{
    /// <summary>启用清屏。</summary>
    ClearColor = 0x01,
    /// <summary>启用深度测试。</summary>
    DepthTest = 0x02,
    /// <summary>启用深度写入。</summary>
    DepthWrite = 0x04,
}

/// <summary>
/// SetRenderPassState 命令数据（32 bytes）——与 C++ NNRenderPassStateData 逐字节对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public unsafe struct RenderPassStateData
{
    /// <summary>RGBA 清屏颜色 [0,1]（16 bytes）。</summary>
    public fixed float ClearColor[4];
    /// <summary>RenderPassFlags 位标志。</summary>
    public uint Flags;
    /// <summary>Stencil 参考值——预留。</summary>
    public uint StencilRef;
    /// <summary>预留。</summary>
    public uint Reserved0;
    /// <summary>预留。</summary>
    public uint Reserved1;
}

/// <summary>
/// 精灵混合模式——与 C++ NNSpriteBlendMode 对齐。
/// </summary>
public enum SpriteBlendMode : uint
{
    /// <summary>Alpha 混合。</summary>
    Alpha = 0,
    /// <summary>叠加。</summary>
    Additive = 1,
    /// <summary>正片叠底。</summary>
    Multiply = 2,
    /// <summary>不透明。</summary>
    Opaque = 3,
    /// <summary>预乘 Alpha。</summary>
    Premultiplied = 4,
}

/// <summary>
/// Sprite flags 位标志——与 C++ NN_SPRITE_FLAG_* 对齐。
/// </summary>
[Flags]
public enum SpriteFlags : uint
{
    /// <summary>水平翻转。</summary>
    FlipX = 0x01,
    /// <summary>垂直翻转。</summary>
    FlipY = 0x02,
}

/// <summary>
/// 单个精灵实例数据（120 bytes）——与 C++ NNSpriteInstance 逐字节对齐。
/// textureHandle = Diligent ITextureView* 编码为 ulong（0 = 白色默认纹理）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public unsafe struct SpriteInstance
{
    /// <summary>WorldMatrix（4x4 列主序，64 bytes）。</summary>
    public fixed float Transform[16];
    /// <summary>Diligent ITextureView* 编码为 ulong（0 = 白色默认纹理）。</summary>
    public ulong TextureHandle;
    /// <summary>RGBA tint [0,1]（16 bytes）。</summary>
    public fixed float Color[4];
    /// <summary>[u0, v0, u1, v1] Atlas UV 区域（16 bytes）。</summary>
    public fixed float UvRect[4];
    /// <summary>渲染层——排序用。</summary>
    public uint Layer;
    /// <summary>层内排序——大的后渲染。</summary>
    public uint SortOrder;
    /// <summary>SpriteBlendMode。</summary>
    public uint BlendMode;
    /// <summary>SpriteFlags 位标志。</summary>
    public uint Flags;
}

/// <summary>
/// DrawSpriteBatch 命令数据头部（16 bytes）——与 C++ NNDrawSpriteBatchData 逐字节对齐。
/// 后跟 spriteCount 个 SpriteInstance。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct DrawSpriteBatchHeader
{
    /// <summary>精灵数量。</summary>
    public uint SpriteCount;
    /// <summary>预留。</summary>
    public uint Reserved0;
    /// <summary>预留。</summary>
    public uint Reserved1;
    /// <summary>预留。</summary>
    public uint Reserved2;
}

/// <summary>
/// RmlUI 文档条目（276 bytes）——与 C++ NNRmlDocumentEntry 逐字节对齐。
/// C# 端从 Friflo ECS 收集 RmlUIDocument 组件，解析 GUID → VFS 路径后传入。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public unsafe struct RmlDocumentEntry
{
    /// <summary>VFS 路径（UTF-8，NUL 终结，最长 255 字符）。</summary>
    public fixed byte AssetPath[256];
    /// <summary>渲染排序——小的在前。</summary>
    public int SortOrder;
    /// <summary>NNRmlUIViewTarget: 0=Scene, 1=Game, 2=Both。</summary>
    public uint ViewTarget;
    /// <summary>C# entity ID——用于 Renderer 内部 Diff。</summary>
    public uint EntityHandle;
    /// <summary>预留：视口 ID（0=默认）。</summary>
    public uint ViewportId;
}

/// <summary>
/// SetRmlDocuments 命令数据头部（16 bytes）——与 C++ NNRmlDocumentsData 逐字节对齐。
/// 后跟 documentCount 个 RmlDocumentEntry。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct RmlDocumentsHeader
{
    /// <summary>文档数量。</summary>
    public uint DocumentCount;
    /// <summary>预留。</summary>
    public uint Reserved0;
    /// <summary>预留。</summary>
    public uint Reserved1;
    /// <summary>预留。</summary>
    public uint Reserved2;
}

// ═══════════════════════════════════════════
//  便利常量
// ═══════════════════════════════════════════

/// <summary>
/// RenderCommands 常量——与 C++ 宏对齐。
/// </summary>
public static class RenderCommandConstants
{
    /// <summary>魔数 "RNDC" (0x524E4443)。</summary>
    public const uint BufferMagic = 0x524E4443u;

    /// <summary>Buffer 头部大小（16 bytes）。</summary>
    public const int BufferHeaderSize = 16;

    /// <summary>命令头部大小（8 bytes）。</summary>
    public const int CommandHeaderSize = 8;

    /// <summary>SetCamera 命令总大小（header 8 + data 152 = 160 bytes）。</summary>
    public const int SetCameraTotalSize = 160;

    /// <summary>SetRenderPassState 命令总大小（header 8 + data 32 = 40 bytes）。</summary>
    public const int SetRenderPassStateTotalSize = 40;

    /// <summary>DrawSpriteBatch 命令头部大小（header 8 + data头部 16 = 24 bytes）。</summary>
    public const int DrawSpriteBatchHeaderSize = 24;

    /// <summary>单个 Sprite 实例大小（120 bytes）。</summary>
    public const int SpriteInstanceSize = 120;

    /// <summary>计算 DrawSpriteBatch 命令的总字节数。</summary>
    public static int DrawSpriteBatchTotalSize(int spriteCount)
        => DrawSpriteBatchHeaderSize + spriteCount * SpriteInstanceSize;

    /// <summary>单个 RmlDocument 条目大小（272 bytes = 256 path + 4×4 fields）。</summary>
    public const int RmlDocumentEntrySize = 272;

    /// <summary>SetRmlDocuments 命令头部大小（header 8 + data头部 16 = 24 bytes）。</summary>
    public const int SetRmlDocumentsHeaderSize = 24;

    /// <summary>assetPath 缓冲区大小（含 NUL 终结符）。</summary>
    public const int RmlDocumentPathSize = 256;

    /// <summary>计算 SetRmlDocuments 命令的总字节数。</summary>
    public static int SetRmlDocumentsTotalSize(int docCount)
        => SetRmlDocumentsHeaderSize + docCount * RmlDocumentEntrySize;
}
