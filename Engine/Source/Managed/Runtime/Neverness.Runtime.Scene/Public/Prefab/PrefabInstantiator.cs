using System.Numerics;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene.Prefab;

/// <summary>
/// 预制体实例化器——将预制体资产实例化为实体。
/// </summary>
public sealed class PrefabInstantiator
{
    private readonly IScene _scene;

    public PrefabInstantiator(IScene scene)
    {
        _scene = scene ?? throw new ArgumentNullException(nameof(scene));
    }

    /// <summary>实例化预制体。</summary>
    /// <param name="prefab">预制体资产。</param>
    /// <param name="position">世界位置。</param>
    /// <param name="rotation">世界旋转。</param>
    /// <param name="scale">世界缩放。</param>
    /// <returns>实例化的根实体。</returns>
    public IEntity Instantiate(PrefabAsset prefab, Vector3? position = null, Quaternion? rotation = null, Vector3? scale = null)
    {
        ArgumentNullException.ThrowIfNull(prefab);

        // 创建根实体
        var rootEntity = _scene.CreateEntity(prefab.Name);
        rootEntity.Add(new TransformComponent
        {
            Position = position ?? Vector3.Zero,
            Rotation = rotation ?? Quaternion.Identity,
            Scale = scale ?? Vector3.One,
        });

        // 添加组件
        foreach (var compData in prefab.Components)
        {
            AddComponentFromData(rootEntity, compData);
        }

        // 创建子实体
        foreach (var childData in prefab.Children)
        {
            CreateChildEntity(rootEntity, childData);
        }

        return rootEntity;
    }

    /// <summary>在指定位置实例化预制体。</summary>
    public IEntity InstantiateAt(PrefabAsset prefab, Vector3 position)
    {
        return Instantiate(prefab, position);
    }

    /// <summary>实例化预制体并设置父实体。</summary>
    public IEntity InstantiateAsChild(PrefabAsset prefab, IEntity parent, Vector3? localPosition = null)
    {
        ArgumentNullException.ThrowIfNull(prefab);
        ArgumentNullException.ThrowIfNull(parent);

        var entity = Instantiate(prefab, localPosition);
        _scene.SetParent(entity, parent);
        return entity;
    }

    // ── 内部方法 ──

    private void CreateChildEntity(IEntity parent, PrefabChildData childData)
    {
        // 创建子实体
        var childEntity = _scene.CreateEntity(childData.Name);
        childEntity.Add(new TransformComponent
        {
            Position = childData.Position,
            Rotation = childData.Rotation,
            Scale = childData.Scale,
        });

        // 添加组件
        foreach (var compData in childData.Components)
        {
            AddComponentFromData(childEntity, compData);
        }

        // 设置父子关系
        _scene.SetParent(childEntity, parent);

        // 递归创建子实体
        foreach (var grandchildData in childData.Children)
        {
            CreateChildEntity(childEntity, grandchildData);
        }
    }

    private void AddComponentFromData(IEntity entity, PrefabComponentData compData)
    {
        // 简化实现：根据类型名称添加组件
        // 实际实现应该使用反射或序列化系统
        switch (compData.TypeName)
        {
            case "TransformComponent":
                // TransformComponent 已经在创建实体时添加
                break;
            case "CameraComponent":
                entity.Add(CameraComponent.DefaultPerspective);
                break;
            case "ScriptComponent":
                entity.Add(new ScriptComponent { ScriptTypeId = 0 });
                break;
            default:
                // 未知组件类型，跳过
                break;
        }
    }
}
