using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 单组件缓存式查询——持有场景句柄和组件 TypeId，按需查询并返回零 GC 视图。
/// 由 <see cref="SceneQueryCache"/> 管理，可复用避免重复分配。
/// </summary>
public sealed class SceneQuery<T> where T : unmanaged
{
    private readonly ulong _sceneHandle;
    private readonly ulong _typeId;
    private readonly uint _componentSize;

    /// <summary>实体句柄缓冲区（按需扩容）。</summary>
    private ulong[] _handleBuffer = [];

    /// <summary>组件数据缓冲区（按需扩容）。</summary>
    private byte[] _dataBuffer = [];

    internal SceneQuery(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
        _typeId = ComponentTypeCache<T>.TypeId;
        _componentSize = (uint)ComponentTypeCache<T>.SizeBytes;
    }

    /// <summary>获取拥有指定组件的实体数量。</summary>
    public int Count => (int)NativeQueryBridge.QueryEntityCount(_sceneHandle, _typeId);

    /// <summary>执行查询并返回当前匹配的零 GC 视图。</summary>
    public SceneView<T> Execute()
    {
        // 1. 获取匹配数量
        var count = NativeQueryBridge.QueryEntityCount(_sceneHandle, _typeId);
        if (count == 0)
        {
            return default;
        }

        // 2. 确保缓冲区足够大
        EnsureHandleBuffer((int)count);
        EnsureDataBuffer((int)count);

        // 3. 获取实体句柄
        var actualCount = NativeQueryBridge.QueryEntities(
            _sceneHandle, _typeId,
            new Span<ulong>(_handleBuffer).Slice(0, (int)count));
        if (actualCount == 0)
        {
            return default;
        }

        // 4. 批量读取组件数据
        var dataSpan = new Span<byte>(_dataBuffer).Slice(0, (int)(actualCount * _componentSize));
        NativeQueryBridge.QueryComponents(
            _sceneHandle, _typeId,
            new ReadOnlySpan<ulong>(_handleBuffer, 0, (int)actualCount),
            dataSpan,
            _componentSize);

        return new SceneView<T>(
            new Span<ulong>(_handleBuffer, 0, (int)actualCount),
            dataSpan,
            (int)actualCount,
            _componentSize);
    }

    private void EnsureHandleBuffer(int count)
    {
        if (_handleBuffer.Length < count)
        {
            _handleBuffer = new ulong[count];
        }
    }

    private void EnsureDataBuffer(int count)
    {
        var needed = count * (int)_componentSize;
        if (_dataBuffer.Length < needed)
        {
            _dataBuffer = new byte[needed];
        }
    }
}

/// <summary>
/// 双组件缓存式查询。
/// </summary>
public sealed class SceneQuery<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    private readonly ulong _sceneHandle;
    private readonly ulong _typeId1;
    private readonly ulong _typeId2;
    private readonly uint _size1;
    private readonly uint _size2;

    private ulong[] _handleBuffer = [];
    private byte[] _buffer1 = [];
    private byte[] _buffer2 = [];

    internal SceneQuery(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
        _typeId1 = ComponentTypeCache<T1>.TypeId;
        _typeId2 = ComponentTypeCache<T2>.TypeId;
        _size1 = (uint)ComponentTypeCache<T1>.SizeBytes;
        _size2 = (uint)ComponentTypeCache<T2>.SizeBytes;
    }

    /// <summary>获取同时拥有两个组件的实体数量。</summary>
    public int Count => (int)NativeQueryBridge.QueryCount2(_sceneHandle, _typeId1, _typeId2);

    /// <summary>执行查询并返回当前匹配的零 GC 视图。</summary>
    public SceneView<T1, T2> Execute()
    {
        // 1. 查询 T1 的实体（先按 T1 过滤，再在 C# 侧过滤 T2）
        //    使用 queryEntities 获取 T1 实体，再 queryComponents 批量获取 T1 + T2
        var count1 = NativeQueryBridge.QueryEntityCount(_sceneHandle, _typeId1);
        if (count1 == 0)
        {
            return default;
        }

        EnsureHandleBuffer((int)count1);

        var actualCount1 = NativeQueryBridge.QueryEntities(
            _sceneHandle, _typeId1,
            new Span<ulong>(_handleBuffer).Slice(0, (int)count1));
        if (actualCount1 == 0)
        {
            return default;
        }

        // 2. 批量获取 T1 组件数据
        EnsureBuffer1((int)actualCount1);
        NativeQueryBridge.QueryComponents(
            _sceneHandle, _typeId1,
            new ReadOnlySpan<ulong>(_handleBuffer, 0, (int)actualCount1),
            new Span<byte>(_buffer1).Slice(0, (int)(actualCount1 * _size1)),
            _size1);

        // 3. 批量获取 T2 组件数据
        EnsureBuffer2((int)actualCount1);
        var result2 = NativeQueryBridge.QueryComponents(
            _sceneHandle, _typeId2,
            new ReadOnlySpan<ulong>(_handleBuffer, 0, (int)actualCount1),
            new Span<byte>(_buffer2).Slice(0, (int)(actualCount1 * _size2)),
            _size2);

        // 4. 过滤掉不拥有 T2 的实体（compacted 数组）
        int writeIdx = 0;
        for (int i = 0; i < (int)actualCount1; i++)
        {
            // 检查 T2 数据是否全零（表示该实体不拥有 T2 组件）
            var offset2 = i * (int)_size2;
            bool hasT2 = !IsAllZero(_buffer2, offset2, (int)_size2);
            if (!hasT2)
            {
                continue;
            }

            if (writeIdx != i)
            {
                // 移动句柄
                _handleBuffer[writeIdx] = _handleBuffer[i];
                // 移动 T1 数据
                Buffer.BlockCopy(_buffer1, i * (int)_size1, _buffer1, writeIdx * (int)_size1, (int)_size1);
                // 移动 T2 数据
                Buffer.BlockCopy(_buffer2, offset2, _buffer2, writeIdx * (int)_size2, (int)_size2);
            }
            writeIdx++;
        }

        if (writeIdx == 0)
        {
            return default;
        }

        return new SceneView<T1, T2>(
            new Span<ulong>(_handleBuffer, 0, writeIdx),
            new Span<byte>(_buffer1, 0, writeIdx * (int)_size1),
            new Span<byte>(_buffer2, 0, writeIdx * (int)_size2),
            writeIdx, _size1, _size2);
    }

    private static bool IsAllZero(byte[] buffer, int offset, int length)
    {
        for (int i = offset; i < offset + length; i++)
        {
            if (buffer[i] != 0) return false;
        }
        return true;
    }

    private void EnsureHandleBuffer(int count)
    {
        if (_handleBuffer.Length < count)
        {
            _handleBuffer = new ulong[count];
        }
    }

    private void EnsureBuffer1(int count)
    {
        var needed = count * (int)_size1;
        if (_buffer1.Length < needed)
        {
            _buffer1 = new byte[needed];
        }
    }

    private void EnsureBuffer2(int count)
    {
        var needed = count * (int)_size2;
        if (_buffer2.Length < needed)
        {
            _buffer2 = new byte[needed];
        }
    }
}
