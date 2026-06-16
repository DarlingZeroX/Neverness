using Neverness.Editor.Core.Public;
using Neverness.Editor.Scene.Private.Cache;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 场景查询服务实现——封装 SceneHierarchyCache。
/// 对外暴露 ISceneQueryService 接口，Controller 不直接访问底层。
/// </summary>
public sealed class SceneQueryServiceImpl : ISceneQueryService
{
    private readonly SceneHierarchyCache _cache = new();
    private IScene? _activeScene;

    /// <summary>当前活跃场景。</summary>
    public IScene? ActiveScene => _activeScene;

    /// <summary>是否有活跃场景。</summary>
    public bool HasActiveScene => _activeScene != null;

    /// <summary>暴露层级缓存（Debug / 诊断用）。</summary>
    public SceneHierarchyCache Cache => _cache;

    /// <summary>设置活跃场景。</summary>
    public void SetActiveScene(IScene? scene)
    {
        _activeScene = scene;
    }

    /// <summary>刷新层级缓存。</summary>
    public bool TryRefreshHierarchy()
    {
        return _cache.TryRefresh(_activeScene);
    }

    /// <summary>获取完整的实体层级树（根节点列表）。</summary>
    public List<EntityNodeData> GetHierarchyTree()
    {
        var allNodes = _cache.AllNodes;
        var rootNodes = new List<EntityNodeData>();

        // 找出所有根节点（Parent == -1）
        for (int i = 0; i < allNodes.Length; i++)
        {
            if (allNodes[i].Parent < 0)
            {
                rootNodes.Add(BuildTreeRecursive(allNodes, i));
            }
        }

        return rootNodes;
    }

    /// <summary>获取所有实体节点（扁平列表）。</summary>
    public List<EntityNodeData> GetAllNodes()
    {
        var allNodes = _cache.AllNodes;
        var result = new List<EntityNodeData>(allNodes.Length);

        for (int i = 0; i < allNodes.Length; i++)
        {
            result.Add(ConvertToNodeData(allNodes[i]));
        }

        return result;
    }

    /// <summary>获取指定实体的信息。</summary>
    public EntityNodeData? GetEntity(int entityId)
    {
        var node = _cache.GetNode(entityId);
        return node != null ? ConvertToNodeData(node) : null;
    }

    /// <summary>获取实体的显示名称。</summary>
    public string GetEntityName(int entityId)
    {
        var node = _cache.GetNode(entityId);
        return node?.Name ?? "";
    }

    /// <summary>检查实体是否存在。</summary>
    public bool EntityExists(int entityId)
    {
        return _cache.GetNode(entityId) != null;
    }

    /// <summary>创建子实体。</summary>
    public IEntity? CreateChildEntity(IEntity? parent)
    {
        if (_activeScene == null) return null;

        var entity = _activeScene.CreateEntity();

        // 设置父节点
        if (parent != null && parent.IsValid)
        {
            _activeScene.SetParent(entity, parent);
        }

        return entity;
    }

    /// <summary>删除实体。</summary>
    public bool DeleteEntity(IEntity entity)
    {
        if (_activeScene == null || entity == null || !entity.IsValid) return false;

        _activeScene.DestroyEntity(entity);
        return true;
    }

    /// <summary>重命名实体。</summary>
    public bool RenameEntity(IEntity entity, string newName)
    {
        if (entity == null || !entity.IsValid) return false;

        entity.Name = newName;
        return true;
    }

    /// <summary>复制实体。</summary>
    public IEntity? DuplicateEntity(IEntity entity)
    {
        if (_activeScene == null || entity == null || !entity.IsValid) return null;

        // 创建新实体
        var newEntity = _activeScene.CreateEntity(entity.Name);

        // TODO: 复制组件数据
        // 需要遍历源实体的所有组件并复制到新实体

        // 设置相同的父节点
        var parent = _activeScene.GetParent(entity);
        if (parent != null && parent.IsValid)
        {
            _activeScene.SetParent(newEntity, parent);
        }

        return newEntity;
    }

    /// <summary>设置实体父节点。</summary>
    public bool ReparentEntity(IEntity entity, IEntity? newParent)
    {
        if (_activeScene == null || entity == null || !entity.IsValid) return false;

        _activeScene.SetParent(entity, newParent ?? throw new ArgumentNullException(nameof(newParent)));
        return true;
    }

    /// <summary>递归构建树结构。</summary>
    private EntityNodeData BuildTreeRecursive(HierarchyNode[] allNodes, int index)
    {
        var node = allNodes[index];
        var children = new List<EntityNodeData>();

        // 查找所有直接子节点
        for (int i = 0; i < allNodes.Length; i++)
        {
            if (allNodes[i].Parent == node.EntityId)
            {
                children.Add(BuildTreeRecursive(allNodes, i));
            }
        }

        return new EntityNodeData
        {
            Id = node.EntityId,
            Name = node.Name,
            ParentId = node.Parent,
            Depth = (int)node.Depth,
            ChildCount = (int)node.ChildCount,
            IsActive = node.IsActive,
            IsPrefabInstance = node.IsPrefabInstance,
            Children = children
        };
    }

    /// <summary>将 HierarchyNode 转换为 EntityNodeData。</summary>
    private static EntityNodeData ConvertToNodeData(HierarchyNode node)
    {
        return new EntityNodeData
        {
            Id = node.EntityId,
            Name = node.Name,
            ParentId = node.Parent,
            Depth = (int)node.Depth,
            ChildCount = (int)node.ChildCount,
            IsActive = node.IsActive,
            IsPrefabInstance = node.IsPrefabInstance,
            Children = new List<EntityNodeData>()
        };
    }
}
