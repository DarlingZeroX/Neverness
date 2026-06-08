namespace Neverness.Editor.Core.Public;

/// <summary>
/// 场景查询服务接口——提供实体层级数据。
/// Scene 模块实现，Controller 通过服务定位器消费。
/// </summary>
public interface ISceneQueryService
{
    /// <summary>当前活跃场景句柄。</summary>
    ulong ActiveSceneHandle { get; }

    /// <summary>是否有活跃场景。</summary>
    bool HasActiveScene { get; }

    /// <summary>获取完整的实体层级树（根节点列表）。</summary>
    List<EntityNodeData> GetHierarchyTree();

    /// <summary>获取所有实体节点（扁平列表）。</summary>
    List<EntityNodeData> GetAllNodes();

    /// <summary>获取指定实体的信息。</summary>
    EntityNodeData? GetEntity(ulong entityHandle);

    /// <summary>获取实体的显示名称。</summary>
    string GetEntityName(ulong entityHandle);

    /// <summary>检查实体是否存在。</summary>
    bool EntityExists(ulong entityHandle);

    /// <summary>创建子实体。</summary>
    ulong CreateChildEntity(ulong parentHandle);

    /// <summary>删除实体。</summary>
    bool DeleteEntity(ulong entityHandle);

    /// <summary>重命名实体。</summary>
    bool RenameEntity(ulong entityHandle, string newName);

    /// <summary>复制实体。</summary>
    ulong DuplicateEntity(ulong entityHandle);

    /// <summary>设置实体父节点。</summary>
    bool ReparentEntity(ulong entityHandle, ulong newParentHandle);

    /// <summary>设置活跃场景。</summary>
    void SetActiveScene(ulong sceneHandle);

    /// <summary>刷新层级缓存（版本轮询 + 增量更新）。</summary>
    bool TryRefreshHierarchy();
}

/// <summary>
/// 实体节点数据（Service 接口专用，与 ViewModel 解耦）。
/// </summary>
public class EntityNodeData
{
    public ulong Handle { get; init; }
    public string Name { get; init; } = "";
    public ulong ParentHandle { get; init; }
    public int Depth { get; init; }
    public int ChildCount { get; init; }
    public bool IsActive { get; init; }
    public bool IsPrefabInstance { get; init; }
    public List<EntityNodeData> Children { get; init; } = new();
    public bool IsRoot => ParentHandle == 0;
    public bool HasChildren => Children.Count > 0;
}
