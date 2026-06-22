using System.Reflection;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// Inspector 服务实现——封装 ComponentInspectorRegistry。
/// 对外暴露 IInspectorService 接口，Controller 不直接访问 Registry。
/// </summary>
public sealed class InspectorServiceImpl : IInspectorService
{
    /// <summary>按 ID 获取实体实例。</summary>
    public IEntity? GetEntityById(int entityId)
    {
        var scene = SceneModuleImp.SceneQueryService?.ActiveScene;
        return scene?.GetEntity(entityId);
    }

    /// <summary>获取指定实体的所有组件信息。</summary>
    public List<ComponentDataInfo> GetEntityComponents(IEntity entity)
    {
        var result = new List<ComponentDataInfo>();
        if (entity == null || !entity.IsValid) return result;

        // 遍历所有注册的 Inspector，检查实体是否拥有该组件
        foreach (var inspector in ComponentInspectorRegistry.Inspectors)
        {
            if (inspector.HasComponent(entity))
            {
                result.Add(new ComponentDataInfo
                {
                    TypeId = inspector.ComponentTypeId,
                    DisplayName = inspector.DisplayName,
                    Order = inspector.Order,
                    CanRemove = true
                });
            }
        }

        // 按 Order 排序
        result.Sort((a, b) => a.Order.CompareTo(b.Order));
        return result;
    }

    /// <summary>检查实体是否拥有指定组件。</summary>
    public bool HasComponent(IEntity entity, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        return inspector?.HasComponent(entity) ?? false;
    }

    /// <summary>添加组件到实体——通过 Inspector.ClrType 反射调用 IEntity.Add&lt;T&gt;。
    /// 优先使用组件的静态 Default 属性（含合理初始值），无则 fallback 到零值。</summary>
    public bool AddComponent(IEntity entity, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        if (inspector == null)
        {
            Console.WriteLine($"[InspectorService] 未找到 TypeId=0x{componentTypeId:X16} 的 Inspector");
            return false;
        }

        var clrType = inspector.ClrType;
        if (clrType == null) return false;

        // 优先取静态 Default 属性（AudioSource.Default、VideoPlayer.Default 等含非零初始值）
        var defaultProp = clrType.GetProperty("Default", BindingFlags.Public | BindingFlags.Static);
        var component = defaultProp?.GetValue(null) ?? Activator.CreateInstance(clrType);
        if (component == null) return false;

        // 反射调用 IEntity.Add<T>(T component)
        var addMethod = typeof(IEntity).GetMethod(nameof(IEntity.Add))?.MakeGenericMethod(clrType);
        if (addMethod == null) return false;

        addMethod.Invoke(entity, [component]);
        return true;
    }

    /// <summary>从实体移除组件。</summary>
    public bool RemoveComponent(IEntity entity, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        if (inspector == null) return false;

        inspector.RemoveComponent(entity);
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
    public string GetEntityName(IEntity entity)
    {
        if (entity == null || !entity.IsValid)
            return "Invalid Entity";

        return entity.Name ?? $"Entity_{entity.Id}";
    }

    /// <summary>获取实体是否激活。</summary>
    public bool IsEntityActive(IEntity entity)
    {
        if (entity == null || !entity.IsValid)
            return false;

        // TODO: 检查实体的激活状态标签
        return true; // 默认激活
    }

    /// <summary>设置实体激活状态。</summary>
    public void SetEntityActive(IEntity entity, bool active)
    {
        // TODO: 设置实体的激活状态标签
        Console.WriteLine($"[InspectorService] SetEntityActive 待实现: {entity.Id} = {active}");
    }

    /// <summary>绘制指定组件的 Inspector 字段。</summary>
    public bool DrawComponentInspector(IEntity entity, ulong componentTypeId)
    {
        var inspector = ComponentInspectorRegistry.GetInspector(componentTypeId);
        if (inspector == null) return false;

        return inspector.DrawInspector(entity);
    }
}
