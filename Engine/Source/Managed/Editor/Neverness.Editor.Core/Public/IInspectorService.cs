using Neverness.Runtime.Scene;

namespace Neverness.Editor.Core.Public;

/// <summary>
/// Inspector 服务接口——提供实体组件数据和操作。
/// Scene 模块实现，Controller 通过服务定位器消费。
/// </summary>
public interface IInspectorService
{
    /// <summary>获取指定实体的所有组件信息。</summary>
    List<ComponentDataInfo> GetEntityComponents(IEntity entity);

    /// <summary>按 ID 获取实体实例（返回 null 表示不存在）。</summary>
    IEntity? GetEntityById(int entityId);

    /// <summary>检查实体是否拥有指定组件。</summary>
    bool HasComponent(IEntity entity, ulong componentTypeId);

    /// <summary>添加组件到实体。</summary>
    bool AddComponent(IEntity entity, ulong componentTypeId);

    /// <summary>从实体移除组件。</summary>
    bool RemoveComponent(IEntity entity, ulong componentTypeId);

    /// <summary>获取所有可用的组件类型（用于"添加组件"菜单）。</summary>
    List<ComponentTypeInfo> GetAvailableComponentTypes();

    /// <summary>获取实体的显示名称。</summary>
    string GetEntityName(IEntity entity);

    /// <summary>获取实体是否激活。</summary>
    bool IsEntityActive(IEntity entity);

    /// <summary>设置实体激活状态。</summary>
    void SetEntityActive(IEntity entity, bool active);

    /// <summary>绘制指定组件的 Inspector 字段（由 IComponentInspector 实现）。</summary>
    /// <returns>是否修改了组件数据。</returns>
    bool DrawComponentInspector(IEntity entity, ulong componentTypeId);
}

/// <summary>
/// 组件数据信息（Service 接口专用）。
/// </summary>
public class ComponentDataInfo
{
    public ulong TypeId { get; init; }
    public string DisplayName { get; init; } = "";
    public int Order { get; init; }
    public bool CanRemove { get; init; } = true;
}

/// <summary>
/// 组件类型信息（用于"添加组件"菜单）。
/// </summary>
public class ComponentTypeInfo
{
    public ulong TypeId { get; init; }
    public string DisplayName { get; init; } = "";
    public string Category { get; init; } = "";
    public bool AlreadyAdded { get; init; }
}
