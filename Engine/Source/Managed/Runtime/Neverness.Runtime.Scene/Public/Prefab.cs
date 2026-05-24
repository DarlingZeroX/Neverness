using System.Reflection;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 便捷包装器——保留原有的 builder 模式 API（<see cref="WithComponent{T}"/>），
/// 内部委托 <see cref="PrefabAsset"/> + <see cref="PrefabInstantiator"/> 完成实例化。
/// </summary>
public sealed class Prefab
{
    /// <summary>Prefab 名称。</summary>
    public string Name { get; }

    /// <summary>模板显示名称（实例化后的默认 DisplayName）。</summary>
    public string TemplateDisplayName { get; }

    /// <summary>底层 Prefab 资产（首次实例化时延迟构建）。</summary>
    public PrefabAsset Asset => _asset ??= BuildAsset();

    private readonly List<Type> _componentTypes = [];
    private PrefabAsset? _asset;

    /// <summary>建立 Prefab 描述。</summary>
    public Prefab(string name, string templateDisplayName = "Entity")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        Name = name;
        TemplateDisplayName = templateDisplayName;
    }

    /// <summary>注册需要自动添加的组件类型。</summary>
    public Prefab WithComponent<T>() where T : struct
    {
        _componentTypes.Add(typeof(T));
        _asset = null; // 标记需重建
        return this;
    }

    /// <summary>
    /// 在指定世界中实例化一个实体（创建 + 添加注册组件）。
    /// </summary>
    /// <param name="world">目标场景世界。</param>
    /// <returns>创建的实体；失败时返回 null。</returns>
    public SceneEntity? Instantiate(SceneWorld world)
    {
        ArgumentNullException.ThrowIfNull(world);

        var handle = SceneNativeBridge.CreateEntity(world.NativeHandle);
        if (handle.Value == 0)
        {
            return null;
        }

        // 逐类型添加组件
        foreach (var type in _componentTypes)
        {
            AddComponentByType(world.NativeHandle, handle, type);
        }

        return new SceneEntity(handle, world.NativeHandle, TemplateDisplayName);
    }

    /// <summary>
    /// 在指定世界中实例化一个实体（兼容旧 API：通过场景句柄创建）。
    /// </summary>
    /// <param name="sceneHandle">场景句柄。</param>
    /// <param name="manager">场景管理器（用于跟踪实体）；为 null 时不跟踪。</param>
    /// <returns>创建的实体；失败时返回 null。</returns>
    public SceneEntity? Instantiate(ulong sceneHandle, SceneManager? manager = null)
    {
        if (sceneHandle == 0)
        {
            return null;
        }

        var handle = SceneNativeBridge.CreateEntity(sceneHandle);
        if (handle.Value == 0)
        {
            return null;
        }

        foreach (var type in _componentTypes)
        {
            AddComponentByType(sceneHandle, handle, type);
        }

        var entity = new SceneEntity(handle, sceneHandle, TemplateDisplayName);
        manager?.TrackEntity(entity);

        return entity;
    }

    /// <summary>通过反射调用泛型 AddComponent（内部使用）。</summary>
    private static void AddComponentByType(ulong sceneHandle, NNEntityHandle handle, Type componentType)
    {
        var method = typeof(SceneNativeBridge)
            .GetMethod(nameof(SceneNativeBridge.AddComponent))!
            .MakeGenericMethod(componentType);
        method.Invoke(null, [sceneHandle, handle]);
    }

    /// <summary>从注册的组件类型构建 Prefab 资产（单实体，无层级）。</summary>
    private PrefabAsset BuildAsset()
    {
        var asset = new PrefabAsset { Name = Name };

        var rootData = new PrefabEntityData
        {
            LocalIndex = 0,
            DisplayName = TemplateDisplayName,
        };

        // 为每个注册的组件类型生成空数据占位（大小由 Marshal.SizeOf 确定）
        foreach (var type in _componentTypes)
        {
            var size = System.Runtime.InteropServices.Marshal.SizeOf(type);
            var typeId = GetTypeId(type);
            rootData.Components[typeId] = new byte[size];
        }

        asset.Entities.Add(rootData);
        return asset;
    }

    private static ulong GetTypeId(Type componentType)
    {
        var attr = componentType.GetCustomAttribute<ComponentIdAttribute>();
        if (attr != null)
        {
            return attr.TypeId;
        }

        // Fallback: FNV-1a of type name
        var name = componentType.Name;
        ulong hash = 14695981039346656037UL;
        foreach (var c in name)
        {
            hash ^= (byte)c;
            hash *= 1099511628211UL;
        }
        return hash;
    }
}
