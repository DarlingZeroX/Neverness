using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 单组件视图——批量获取匹配实体的组件数据。
/// ref struct 保证零 GC、零 boxing，直接在 Span 上操作。
/// </summary>
public ref struct SceneView<T> where T : unmanaged
{
    private readonly Span<ulong> _entityHandles;
    private readonly Span<byte> _componentBuffer;
    private readonly int _count;
    private readonly uint _componentSize;

    internal SceneView(Span<ulong> handles, Span<byte> buffer, int count, uint componentSize)
    {
        _entityHandles = handles;
        _componentBuffer = buffer;
        _count = count;
        _componentSize = componentSize;
    }

    /// <summary>实际匹配的实体数量。</summary>
    public readonly int Count => _count;

    /// <summary>获取指定索引的实体句柄。</summary>
    public readonly NNEntityHandle GetEntity(int index) => new(_entityHandles[index]);

    /// <summary>获取指定索引的组件 ref。</summary>
    public readonly ref T GetComponent(int index)
    {
        var offset = (int)(_componentSize * index);
        return ref MemoryMarshal.AsRef<T>(_componentBuffer.Slice(offset, (int)_componentSize));
    }

    public readonly SceneViewEnumerator<T> GetEnumerator() => new(this);
}

/// <summary>
/// 单组件视图枚举器——零 GC，支持 foreach(ref var entry in view)。
/// </summary>
public ref struct SceneViewEnumerator<T> where T : unmanaged
{
    private readonly SceneView<T> _view;
    private int _index;

    internal SceneViewEnumerator(SceneView<T> view)
    {
        _view = view;
        _index = -1;
    }

    public bool MoveNext() => ++_index < _view.Count;

    public readonly SceneViewEntry<T> Current => new(_view.GetEntity(_index), ref _view.GetComponent(_index));
}

/// <summary>
/// 单组件视图条目——提供 Entity 句柄和组件 ref 访问。
/// </summary>
public readonly ref struct SceneViewEntry<T> where T : unmanaged
{
    public readonly NNEntityHandle Entity;
    private readonly ref T _component;

    public ref T Component => ref _component;

    internal SceneViewEntry(NNEntityHandle entity, ref T component)
    {
        Entity = entity;
        _component = ref component;
    }
}
