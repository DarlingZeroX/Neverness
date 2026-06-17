using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Inspector;

/// <summary>
/// 通用组件检查器——仅实现 HasComponent / RemoveComponent / TypeId。
/// 不绘制 UI（Avalonia 前端由 AvaloniaComponentInspectorRegistry 负责绘制）。
/// 用于填充 ComponentInspectorRegistry，使 InspectorServiceImpl.GetEntityComponents 能枚举组件。
/// </summary>
internal sealed class GenericComponentInspector<T> : IComponentInspector where T : struct, IComponent
{
    private readonly ulong _typeId;
    private readonly string _displayName;
    private readonly int _order;

    public GenericComponentInspector(ulong typeId, string displayName, int order = 1000)
    {
        _typeId = typeId;
        _displayName = displayName;
        _order = order;
    }

    public ulong ComponentTypeId => _typeId;
    public string DisplayName => _displayName;
    public Type ClrType => typeof(T);
    public int Order => _order;

    public bool HasComponent(IEntity entity) => entity.Has<T>();
    public void RemoveComponent(IEntity entity) => entity.Remove<T>();

    /// <summary>此检查器不绘制 UI，返回 false。</summary>
    public bool DrawInspector(IEntity entity) => false;
}
