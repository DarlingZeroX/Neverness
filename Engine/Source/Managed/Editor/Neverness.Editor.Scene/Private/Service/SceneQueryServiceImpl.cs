using Neverness.Editor.Core.Public;
using Neverness.Editor.Scene.Private.Cache;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 场景查询服务实现——封装 SceneHierarchyCache 和 EditorSceneNativeBridge。
/// 对外暴露 ISceneQueryService 接口，Controller 不直接访问 Native。
/// </summary>
public sealed class SceneQueryServiceImpl : ISceneQueryService
{
    private readonly SceneHierarchyCache _cache = new();
    private ulong _activeSceneHandle;

    /// <summary>当前活跃场景句柄。</summary>
    public ulong ActiveSceneHandle => _activeSceneHandle;

    /// <summary>是否有活跃场景。</summary>
    public bool HasActiveScene => _activeSceneHandle != 0;

    /// <summary>暴露层级缓存（Debug / 诊断用）。</summary>
    public SceneHierarchyCache Cache => _cache;

    /// <summary>设置活跃场景。</summary>
    public void SetActiveScene(ulong sceneHandle)
    {
        _activeSceneHandle = sceneHandle;
    }

    /// <summary>刷新层级缓存。</summary>
    public bool TryRefreshHierarchy()
    {
        if (_activeSceneHandle == 0) return false;
        return _cache.TryRefresh(_activeSceneHandle);
    }

    /// <summary>获取完整的实体层级树（根节点列表）。</summary>
    public List<EntityNodeData> GetHierarchyTree()
    {
        var allNodes = _cache.AllNodes;
        var rootNodes = new List<EntityNodeData>();

        // 找出所有根节点（Parent == 0）
        for (int i = 0; i < allNodes.Length; i++)
        {
            if (allNodes[i].Parent == 0)
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
    public EntityNodeData? GetEntity(ulong entityHandle)
    {
        var node = _cache.GetNode(entityHandle);
        return node != null ? ConvertToNodeData(node) : null;
    }

    /// <summary>获取实体的显示名称。</summary>
    public string GetEntityName(ulong entityHandle)
    {
        var node = _cache.GetNode(entityHandle);
        return node?.Name ?? "";
    }

    /// <summary>检查实体是否存在。</summary>
    public bool EntityExists(ulong entityHandle)
    {
        return _cache.GetNode(entityHandle) != null;
    }

    /// <summary>创建子实体。</summary>
    public ulong CreateChildEntity(ulong parentHandle)
    {
        if (_activeSceneHandle == 0) return 0;

        var handle = SceneNativeBridge.CreateEntity(_activeSceneHandle);
        if (handle.Value == 0) return 0;

        // 设置父节点
        if (parentHandle != 0)
        {
            SceneNativeBridge.SetParent(_activeSceneHandle, handle, new NNEntityHandle(parentHandle));
        }

        return handle.Value;
    }

    /// <summary>删除实体。</summary>
    public bool DeleteEntity(ulong entityHandle)
    {
        if (_activeSceneHandle == 0) return false;
        return SceneNativeBridge.DestroyEntity(_activeSceneHandle, new NNEntityHandle(entityHandle)) == NNSceneResult.Ok;
    }

    /// <summary>重命名实体。</summary>
    /// <remarks>
    /// Native API 没有直接的重命名方法。
    /// 实体名称存储在快照的 NamePool 中，目前无法从 C# 端修改。
    /// 返回 false 表示操作不支持。
    /// </remarks>
    public bool RenameEntity(ulong entityHandle, string newName)
    {
        // Native API 不支持重命名实体
        // 名称存储在 Native 端的快照中，没有暴露修改接口
        Console.WriteLine($"[SceneQueryService] RenameEntity 不支持: {entityHandle} -> {newName} (Native API 无重命名接口)");
        return false;
    }

    /// <summary>复制实体。</summary>
    /// <remarks>
    /// 通过以下步骤实现：
    /// 1. 获取源实体的所有组件类型和数据
    /// 2. 创建新实体
    /// 3. 复制所有组件到新实体
    /// 4. 设置相同的父节点
    /// </remarks>
    public unsafe ulong DuplicateEntity(ulong entityHandle)
    {
        if (_activeSceneHandle == 0) return 0;

        try
        {
            // 1. 获取源实体的组件数量
            uint componentCount = EditorSceneNativeBridge.GetEntityComponentCount(_activeSceneHandle, entityHandle);
            if (componentCount == 0)
            {
                // 没有组件，只创建新实体
                var newHandle = SceneNativeBridge.CreateEntity(_activeSceneHandle);
                return newHandle.Value;
            }

            // 2. 获取源实体的所有组件信息
            Span<NNEditorComponentInfo> componentInfos = stackalloc NNEditorComponentInfo[(int)componentCount];
            uint written = EditorSceneNativeBridge.GetEntityComponents(_activeSceneHandle, entityHandle, componentInfos);
            if (written == 0) return 0;

            // 3. 创建新实体
            var newEntity = SceneNativeBridge.CreateEntity(_activeSceneHandle);
            if (newEntity.Value == 0) return 0;

            // 4. 复制每个组件
            for (int i = 0; i < (int)written; i++)
            {
                ref readonly var compInfo = ref componentInfos[i];

                // 获取组件数据大小
                uint dataSize = compInfo.FieldCount * 64; // 估算大小
                if (dataSize == 0) continue;

                // 分配缓冲区
                byte[] buffer = new byte[dataSize];

                // 获取组件原始数据
                uint actualSize;
                fixed (byte* pBuf = buffer)
                {
                    actualSize = EditorSceneNativeBridge.GetComponentRawData(
                        _activeSceneHandle, entityHandle, compInfo.TypeId, pBuf, dataSize);
                }

                if (actualSize == 0) continue;

                // 设置组件数据到新实体
                var result = SceneNativeBridge.SetComponentData(
                    _activeSceneHandle, newEntity, compInfo.TypeId, buffer.AsSpan(0, (int)actualSize));

                if (result != NNSceneResult.Ok)
                {
                    Console.WriteLine($"[SceneQueryService] 复制组件失败: TypeId={compInfo.TypeId}, Result={result}");
                }
            }

            // 5. 设置相同的父节点
            var parentNode = _cache.GetNode(entityHandle);
            if (parentNode != null && parentNode.Parent != 0)
            {
                SceneNativeBridge.SetParent(_activeSceneHandle, newEntity, new NNEntityHandle(parentNode.Parent));
            }

            Console.WriteLine($"[SceneQueryService] DuplicateEntity 成功: {entityHandle} -> {newEntity.Value}");
            return newEntity.Value;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SceneQueryService] DuplicateEntity 失败: {entityHandle} -> {ex.Message}");
            return 0;
        }
    }

    /// <summary>设置实体父节点。</summary>
    public bool ReparentEntity(ulong entityHandle, ulong newParentHandle)
    {
        if (_activeSceneHandle == 0) return false;
        return SceneNativeBridge.SetParent(
            _activeSceneHandle,
            new NNEntityHandle(entityHandle),
            new NNEntityHandle(newParentHandle)) == NNSceneResult.Ok;
    }

    /// <summary>递归构建树结构。</summary>
    private EntityNodeData BuildTreeRecursive(HierarchyNode[] allNodes, int index)
    {
        var node = allNodes[index];
        var children = new List<EntityNodeData>();

        // 查找所有直接子节点
        for (int i = 0; i < allNodes.Length; i++)
        {
            if (allNodes[i].Parent == node.Entity)
            {
                children.Add(BuildTreeRecursive(allNodes, i));
            }
        }

        return new EntityNodeData
        {
            Handle = node.Entity,
            Name = node.Name,
            ParentHandle = node.Parent,
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
            Handle = node.Entity,
            Name = node.Name,
            ParentHandle = node.Parent,
            Depth = (int)node.Depth,
            ChildCount = (int)node.ChildCount,
            IsActive = node.IsActive,
            IsPrefabInstance = node.IsPrefabInstance,
            Children = new List<EntityNodeData>() // 扁平列表不包含子节点
        };
    }
}
