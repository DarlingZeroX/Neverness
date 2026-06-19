using System.Buffers.Binary;
using System.Runtime.InteropServices;

namespace Neverness.Rendering.Diligent.Commands;

/// <summary>
/// 渲染命令缓冲区构建器——C# 端序列化 RenderCommands 为 Flat Buffer。
///
/// 使用方式：
/// <code>
/// var buffer = new RenderCommandBuffer();
/// buffer.AddSetCamera(viewMatrix, projMatrix, width, height);
/// buffer.AddSetRenderPassState(clearColor, RenderPassFlags.ClearColor);
/// buffer.AddDrawSpriteBatch(sprites);
/// byte[] data = buffer.Build();
/// </code>
///
/// 设计：
/// - 内部用 MemoryStream 累积命令，Build() 时写入 BufferHeader
/// - 所有字段手动 little-endian 写入，保证 ABI 稳定
/// - 一次性使用：Build() 后不可再 Add
/// </summary>
public sealed class RenderCommandBuffer
{
    private readonly MemoryStream _stream = new();
    private uint _commandCount;
    private bool _built;

    /// <summary>
    /// 添加 SetCamera 命令。
    /// </summary>
    /// <param name="viewMatrix">View 矩阵（16 floats，列主序）</param>
    /// <param name="projectionMatrix">Projection 矩阵（16 floats，列主序）</param>
    /// <param name="viewportWidth">视口宽度</param>
    /// <param name="viewportHeight">视口高度</param>
    /// <param name="nearPlane">近平面</param>
    /// <param name="farPlane">远平面</param>
    public void AddSetCamera(
        ReadOnlySpan<float> viewMatrix,
        ReadOnlySpan<float> projectionMatrix,
        float viewportWidth,
        float viewportHeight,
        float nearPlane,
        float farPlane,
        float orthoWidth = 0f,
        float orthoHeight = 0f)
    {
        ObjectDisposedException.ThrowIf(_built, this);

        // 写入命令头
        WriteCommandHeader((uint)RenderCommandType.SetCamera, RenderCommandConstants.SetCameraTotalSize);

        // 手动写入 SetCameraData（144 bytes）
        WriteFloatSpan(viewMatrix[..16]);          // 64 bytes
        WriteFloatSpan(projectionMatrix[..16]);    // 64 bytes
        WriteFloat(viewportWidth);                 // 4 bytes
        WriteFloat(viewportHeight);                // 4 bytes
        WriteFloat(nearPlane);                     // 4 bytes
        WriteFloat(farPlane);                      // 4 bytes
        WriteFloat(orthoWidth);                    // 4 bytes
        WriteFloat(orthoHeight);                   // 4 bytes
        _commandCount++;
    }

    /// <summary>
    /// 添加 SetRenderPassState 命令。
    /// </summary>
    /// <param name="clearColor">RGBA 清屏颜色（4 floats）</param>
    /// <param name="flags">渲染 Pass 标志位</param>
    public void AddSetRenderPassState(
        ReadOnlySpan<float> clearColor,
        RenderPassFlags flags)
    {
        ObjectDisposedException.ThrowIf(_built, this);

        // 写入命令头
        WriteCommandHeader((uint)RenderCommandType.SetRenderPassState, RenderCommandConstants.SetRenderPassStateTotalSize);

        // 手动写入 RenderPassStateData（32 bytes）
        WriteFloatSpan(clearColor[..4]);   // 16 bytes
        WriteUInt((uint)flags);            // 4 bytes
        WriteUInt(0);                      // stencilRef (预留)
        WriteUInt(0);                      // reserved0
        WriteUInt(0);                      // reserved1
        _commandCount++;
    }

    /// <summary>
    /// 添加 DrawSpriteBatch 命令。
    /// </summary>
    /// <param name="sprites">精灵实例数组</param>
    public void AddDrawSpriteBatch(ReadOnlySpan<SpriteInstance> sprites)
    {
        ObjectDisposedException.ThrowIf(_built, this);

        int totalSize = RenderCommandConstants.DrawSpriteBatchTotalSize(sprites.Length);
        WriteCommandHeader((uint)RenderCommandType.DrawSpriteBatch, totalSize);

        // 写入 DrawSpriteBatchHeader（16 bytes）
        WriteUInt((uint)sprites.Length);  // spriteCount
        WriteUInt(0);                     // reserved0
        WriteUInt(0);                     // reserved1
        WriteUInt(0);                     // reserved2

        // 写入每个 SpriteInstance（120 bytes）
        for (int i = 0; i < sprites.Length; i++)
        {
            WriteSpriteInstance(sprites[i]);
        }

        _commandCount++;
    }

    /// <summary>
    /// 添加 SetRmlDocuments 命令。
    /// </summary>
    /// <param name="entries">RmlUI 文档条目数组</param>
    public void AddSetRmlDocuments(ReadOnlySpan<RmlDocumentEntry> entries)
    {
        ObjectDisposedException.ThrowIf(_built, this);

        int totalSize = RenderCommandConstants.SetRmlDocumentsTotalSize(entries.Length);
        WriteCommandHeader((uint)RenderCommandType.SetRmlDocuments, totalSize);

        // 写入 RmlDocumentsHeader（16 bytes）
        WriteUInt((uint)entries.Length);  // documentCount
        WriteUInt(0);                     // reserved0
        WriteUInt(0);                     // reserved1
        WriteUInt(0);                     // reserved2

        // 写入每个 RmlDocumentEntry（276 bytes）
        for (int i = 0; i < entries.Length; i++)
        {
            WriteRmlDocumentEntry(entries[i]);
        }

        _commandCount++;
    }

    /// <summary>
    /// 构建最终的命令缓冲区（含 BufferHeader）。
    /// 返回的字节数组可直接传给 C++ 的 RenderViewportCommands。
    /// </summary>
    public byte[] Build()
    {
        if (_built)
            throw new InvalidOperationException("RenderCommandBuffer 已经 Build 过，不可重复调用。");

        _built = true;

        int dataLen = (int)_stream.Length;
        int totalBytes = RenderCommandConstants.BufferHeaderSize + dataLen;

        var result = new byte[totalBytes];

        // 写入 BufferHeader（16 bytes）
        WriteUIntTo(result, 0, RenderCommandConstants.BufferMagic);
        WriteUIntTo(result, 4, _commandCount);
        WriteUIntTo(result, 8, (uint)totalBytes);
        WriteUIntTo(result, 12, 0);

        // 写入命令数据
        _stream.Position = 0;
        _stream.Read(result, RenderCommandConstants.BufferHeaderSize, dataLen);

        return result;
    }

    /// <summary>
    /// 获取当前缓冲区大小（不含 BufferHeader，仅命令数据）。
    /// </summary>
    public int CurrentDataSize => (int)_stream.Length;

    /// <summary>
    /// 获取当前命令数量。
    /// </summary>
    public uint CommandCount => _commandCount;

    // ═══════════════════════════════════════════
    //  内部写入方法（全部 little-endian）
    // ═══════════════════════════════════════════

    private void WriteCommandHeader(uint type, int totalSize)
    {
        WriteUInt(type);
        WriteUInt((uint)totalSize);
    }

    private void WriteUInt(uint value)
    {
        Span<byte> buf = stackalloc byte[4];
        BinaryPrimitives.WriteUInt32LittleEndian(buf, value);
        _stream.Write(buf);
    }

    private void WriteFloat(float value)
    {
        Span<byte> buf = stackalloc byte[4];
        BinaryPrimitives.WriteSingleLittleEndian(buf, value);
        _stream.Write(buf);
    }

    private void WriteFloatSpan(ReadOnlySpan<float> values)
    {
        Span<byte> buf = stackalloc byte[4];
        for (int i = 0; i < values.Length; i++)
        {
            BinaryPrimitives.WriteSingleLittleEndian(buf, values[i]);
            _stream.Write(buf);
        }
    }

    private void WriteSpriteInstance(in SpriteInstance sprite)
    {
        unsafe
        {
            // Transform（16 floats = 64 bytes）
            for (int i = 0; i < 16; i++)
                WriteFloat(sprite.Transform[i]);

            // TextureHandle（8 bytes）
            Span<byte> buf8 = stackalloc byte[8];
            BinaryPrimitives.WriteUInt64LittleEndian(buf8, sprite.TextureHandle);
            _stream.Write(buf8);

            // Color（4 floats = 16 bytes）
            for (int i = 0; i < 4; i++)
                WriteFloat(sprite.Color[i]);

            // UvRect（4 floats = 16 bytes）
            for (int i = 0; i < 4; i++)
                WriteFloat(sprite.UvRect[i]);

            // Layer, SortOrder, BlendMode, Flags（4 * 4 = 16 bytes）
            WriteUInt(sprite.Layer);
            WriteUInt(sprite.SortOrder);
            WriteUInt(sprite.BlendMode);
            WriteUInt(sprite.Flags);
        }
    }

    private void WriteRmlDocumentEntry(in RmlDocumentEntry entry)
    {
        unsafe
        {
            // AssetPath（256 bytes，UTF-8 NUL-terminated）
            Span<byte> pathBuf = stackalloc byte[256];
            pathBuf.Clear();
            fixed (byte* src = entry.AssetPath)
            {
                // 复制有效字节（NUL 终结由 fixed 数组保证）
                for (int i = 0; i < 256; i++)
                    pathBuf[i] = src[i];
            }
            _stream.Write(pathBuf);

            // SortOrder, ViewTarget, EntityHandle, ViewportId（4 * 4 = 16 bytes）
            WriteUInt((uint)entry.SortOrder);
            WriteUInt(entry.ViewTarget);
            WriteUInt(entry.EntityHandle);
            WriteUInt(entry.ViewportId);
        }
    }

    private static void WriteUIntTo(byte[] buffer, int offset, uint value)
    {
        BinaryPrimitives.WriteUInt32LittleEndian(buffer.AsSpan(offset, 4), value);
    }
}
