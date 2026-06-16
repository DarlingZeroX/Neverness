namespace Neverness.Runtime.ImageCodec;

/// <summary>
/// 紋素格式（引擎級，不依賴任何 GPU API）。
///
/// 與 C++ NNImageFormat 枚舉一致。
/// </summary>
public enum ImageFormat : uint
{
    Unknown = 0,

    // LDR 8-bit 整型格式
    R8,
    RG8,
    RGB8,
    RGBA8,

    // HDR 16-bit 浮點格式
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,

    // HDR 32-bit 浮點格式
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
}
