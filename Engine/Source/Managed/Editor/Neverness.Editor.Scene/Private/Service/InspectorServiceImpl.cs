using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// Inspector 服务实现——封装 ComponentInspectorRegistry 和 EditorSceneNativeBridge。
/// 对外暴露 IInspectorService 接口，Controller 不直接访问 Registry。
/// </summary>
public sealed class InspectorServiceImpl : IInspectorService
{
    /// <summary>获取指定实体的所有组件信息。</summary>
    public List<ComponentDataInfo> GetEntityComponents(ulong sceneHandle, ulong entityHandle)
    {
        var result = new List<ComponentDataInfo>();
        var entity = new NNEntityHandle(entityHandle);

        // 遍历所有已注册的 Inspector，检查实体是否拥有该组件
        foreach (var inspector in ComponentInspectorRegistry.Inspectors)
        {
            if (inspector.HasComponent(sceneHandle, entity))
            {
                result.Add(new ComponentDataInfo
                {
                    TypeId = inspector.ComponentTypeId,
                    DisplayName = inspector.DisplayName,
                    Order = inspector.Order,
                    CanRemove = true // 大部分组件可移除
                });
            }
        }

        // 按 Order 排序
        result.Sort((a, b) => a.Order.CompareTo(b.Order));
        return result;
    }

    /// <summary>检查实体是否拥有指定组件。</summary>
    public bool HasComponent(ulong sceneHandle, ulong entityHandle, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        return inspector?.HasComponent(sceneHandle, new NNEntityHandle(entityHandle)) ?? false;
    }

    /// <summary>添加组件到实体。</summary>
    public bool AddComponent(ulong sceneHandle, ulong entityHandle, ulong componentTypeId)
    {
        return SceneNativeBridge.AddComponent(sceneHandle, new NNEntityHandle(entityHandle), componentTypeId) == NNSceneResult.Ok;
    }

    /// <summary>从实体移除组件。</summary>
    public bool RemoveComponent(ulong sceneHandle, ulong entityHandle, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        if (inspector == null) return false;

        inspector.RemoveComponent(sceneHandle, new NNEntityHandle(entityHandle));
        return true;
    }

    /// <summary>获取所有可用的组件类型（用于"添加组件"菜单）。</summary>
    public List<ComponentTypeInfo> GetAvailableComponentTypes()
    {
        var result = new List<ComponentTypeInfo>();

        foreach (var inspector in ComponentInspectorRegistry.Inspectors)
        {
            result.Add(new ComponentTypeInfo
            {
                TypeId = inspector.ComponentTypeId,
                DisplayName = inspector.DisplayName,
                Category = "", // 可以后续扩展分类
                AlreadyAdded = false // 需要根据实体状态判断
            });
        }

        return result;
    }

    /// <summary>获取实体的显示名称。</summary>
    /// <remarks>通过 ISceneQueryService 从 HierarchyCache 获取。</remarks>
    public string GetEntityName(ulong sceneHandle, ulong entityHandle)
    {
        try
        {
            var context = EditorCoreModule.Context;
            if (context.TryGetService<ISceneQueryService>(out var sceneQueryService))
            {
                var entity = sceneQueryService.GetEntity(entityHandle);
                if (entity != null)
                {
                    return entity.Name;
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[InspectorService] GetEntityName 失败: {ex.Message}");
        }

        return $"Entity 0x{entityHandle:X16}";
    }

    /// <summary>获取实体是否激活。</summary>
    /// <remarks>通过 ISceneQueryService 从 HierarchyCache 获取。</remarks>
    public bool IsEntityActive(ulong sceneHandle, ulong entityHandle)
    {
        try
        {
            var context = EditorCoreModule.Context;
            if (context.TryGetService<ISceneQueryService>(out var sceneQueryService))
            {
                var entity = sceneQueryService.GetEntity(entityHandle);
                if (entity != null)
                {
                    return entity.IsActive;
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[InspectorService] IsEntityActive 失败: {ex.Message}");
        }

        return true; // 默认激活
    }

    /// <summary>设置实体激活状态。</summary>
    /// <remarks>
    /// Native API 没有直接的 SetActive 方法。
    /// 实体激活状态存储在快照的 Flags 中，目前无法从 C# 端修改。
    /// </remarks>
    public void SetEntityActive(ulong sceneHandle, ulong entityHandle, bool active)
    {
        // Native API 不支持设置实体激活状态
        // Flags 存储在 Native 端的快照中，没有暴露修改接口
        Console.WriteLine($"[InspectorService] SetEntityActive 不支持: {entityHandle} = {active} (Native API 无接口)");
    }

    /// <summary>绘制指定组件的 Inspector 字段。</summary>
    public bool DrawComponentInspector(ulong sceneHandle, ulong entityHandle, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        if (inspector == null) return false;

        return inspector.DrawInspector(sceneHandle, new NNEntityHandle(entityHandle));
    }
}
