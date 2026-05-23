using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 双组件视图——批量获取同时拥有 T1 和 T2 的实体及其组件数据。
/// ref struct 保证零 GC。
/// </summary>
public ref struct SceneView<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    private readonly Span<ulong> _entityHandles;
    private readonly Span<byte> _buffer1;
    private readonly Span<byte> _buffer2;
    private readonly int _count;
    private readonly uint _size1;
    private readonly uint _size2;

    internal SceneView(
        Span<ulong> handles,
        Span<byte> buffer1, Span<byte> buffer2,
        int count, uint size1, uint size2)
    {
        _entityHandles = handles;
        _buffer1 = buffer1;
        _buffer2 = buffer2;
        _count = count;
        _size1 = size1;
        _size2 = size2;
    }

    /// <summary>实际匹配的实体数量。</summary>
    public readonly int Count => _count;

    /// <summary>获取指定索引的实体句柄。</summary>
    public readonly NNEntityHandle GetEntity(int index) => new(_entityHandles[index]);

    /// <summary>获取指定索引的第一个组件 ref。</summary>
    public readonly ref T1 GetComponent1(int index)
    {
        var offset = (int)(_size1 * index);
        return ref MemoryMarshal.AsRef<T1>(_buffer1.Slice(offset, (int)_size1));
    }

    /// <summary>获取指定索引的第二个组件 ref。</summary>
    public readonly ref T2 GetComponent2(int index)
    {
        var offset = (int)(_size2 * index);
        return ref MemoryMarshal.AsRef<T2>(_buffer2.Slice(offset, (int)_size2));
    }

    public readonly SceneViewEnumerator2<T1, T2> GetEnumerator() => new(this);
}

/// <summary>
/// 双组件视图枚举器。
/// </summary>
public ref struct SceneViewEnumerator2<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    private readonly SceneView<T1, T2> _view;
    private int _index;

    internal SceneViewEnumerator2(SceneView<T1, T2> view)
    {
        _view = view;
        _index = -1;
    }

    public bool MoveNext() => ++_index < _view.Count;

    public readonly SceneViewEntry2<T1, T2> Current =>
        new(_view.GetEntity(_index), ref _view.GetComponent1(_index), ref _view.GetComponent2(_index));
}

/// <summary>
/// 双组件视图条目。
/// </summary>
public readonly ref struct SceneViewEntry2<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    public readonly NNEntityHandle Entity;
    private readonly ref T1 _c1;
    private readonly ref T2 _c2;

    public ref T1 Component1 => ref _c1;
    public ref T2 Component2 => ref _c2;

    internal SceneViewEntry2(NNEntityHandle entity, ref T1 c1, ref T2 c2)
    {
        Entity = entity;
        _c1 = ref c1;
        _c2 = ref c2;
    }
}
