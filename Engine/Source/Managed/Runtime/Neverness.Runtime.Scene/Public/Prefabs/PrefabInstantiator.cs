using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 实例化器——从 <see cref="PrefabAsset"/> 创建运行时实体子图，
/// 生成 <see cref="PrefabInstance"/> 跟踪映射与覆盖。
/// </summary>
public static class PrefabInstantiator
{
    /// <summary>
    /// 在指定世界中实例化 Prefab 资产。
    /// 创建所有实体、写入组件数据、建立层级关系。
    /// </summary>
    /// <param name="asset">源 Prefab 资产。</param>
    /// <param name="world">目标场景世界。</param>
    /// <returns>Prefab 实例（含实体映射）；资产为空或世界无效时返回 null。</returns>
    public static PrefabInstance? Instantiate(PrefabAsset asset, SceneWorld world)
    {
        ArgumentNullException.ThrowIfNull(asset);
        ArgumentNullException.ThrowIfNull(world);

        if (asset.Entities.Count == 0)
        {
            return null;
        }

        var instance = new PrefabInstance(asset);
        var localToHandle = new Dictionary<int, NNEntityHandle>(asset.Entities.Count);

        // 第一遍：创建所有实体并写入组件数据
        foreach (var entityData in asset.Entities)
        {
            var handle = SceneNativeBridge.CreateEntity(world.NativeHandle);
            if (handle.Value == 0)
            {
                // 创建失败，回滚已创建的实体
                RollbackCreatedEntities(world, localToHandle);
                return null;
            }

            localToHandle[entityData.LocalIndex] = handle;

            // 写入组件数据
            foreach (var (typeId, data) in entityData.Components)
            {
                SceneNativeBridge.SetComponentData(world.NativeHandle, handle, typeId, data);
            }
        }

        // 第二遍：建立层级关系
        foreach (var entityData in asset.Entities)
        {
            if (entityData.ParentIndex < 0)
            {
                continue;
            }

            if (localToHandle.TryGetValue(entityData.ParentIndex, out var parentHandle))
            {
                var childHandle = localToHandle[entityData.LocalIndex];
                SceneNativeBridge.SetParent(world.NativeHandle, childHandle, parentHandle);
            }
        }

        // 填充实例映射
        instance.RootEntity = localToHandle[0];
        instance.SetInstanceMap(localToHandle);

        return instance;
    }

    /// <summary>
    /// 对已实例化的 Prefab 应用覆盖列表。
    /// 支持全部 5 种覆盖类型。
    /// </summary>
    /// <param name="instance">Prefab 实例。</param>
    /// <param name="world">场景世界。</param>
    public static void ApplyOverrides(PrefabInstance instance, SceneWorld world)
    {
        ArgumentNullException.ThrowIfNull(instance);
        ArgumentNullException.ThrowIfNull(world);

        foreach (var ovr in instance.Overrides)
        {
            var handle = instance.GetEntity(ovr.EntityLocalIndex);
            if (handle.Value == 0)
            {
                continue;
            }

            switch (ovr.Type)
            {
                case PrefabOverrideType.PropertyModified:
                    if (ovr.NewValue != null)
                    {
                        SceneNativeBridge.SetComponentData(
                            world.NativeHandle, handle, ovr.ComponentTypeId, ovr.NewValue);
                    }
                    break;

                case PrefabOverrideType.ComponentAdded:
                    SceneNativeBridge.AddComponent(world.NativeHandle, handle, ovr.ComponentTypeId);
                    if (ovr.NewValue != null)
                    {
                        SceneNativeBridge.SetComponentData(
                            world.NativeHandle, handle, ovr.ComponentTypeId, ovr.NewValue);
                    }
                    break;

                case PrefabOverrideType.ComponentRemoved:
                    SceneNativeBridge.RemoveComponent(world.NativeHandle, handle, ovr.ComponentTypeId);
                    break;

                case PrefabOverrideType.ChildAdded:
                    // 创建新子实体并挂载到父实体
                    var childHandle = SceneNativeBridge.CreateEntity(world.NativeHandle);
                    if (childHandle.Value != 0)
                    {
                        SceneNativeBridge.SetParent(world.NativeHandle, childHandle, handle);
                        // 写入组件数据（如有）
                        if (ovr.NewValue != null && ovr.NewValue.Length >= sizeof(ulong))
                        {
                            // NewValue 前 8 字节为 componentTypeId，后续为组件数据
                            var typeId = BitConverter.ToUInt64(ovr.NewValue, 0);
                            if (ovr.NewValue.Length > sizeof(ulong))
                            {
                                var componentData = ovr.NewValue.AsSpan(sizeof(ulong));
                                SceneNativeBridge.SetComponentData(
                                    world.NativeHandle, childHandle, typeId, componentData);
                            }
                        }
                    }
                    break;

                case PrefabOverrideType.ChildRemoved:
                    // 销毁指定子实体（ChildIndex 编码在 ComponentTypeId 中）
                    var childToRemove = instance.GetEntity((int)ovr.ComponentTypeId);
                    if (childToRemove.Value != 0)
                    {
                        SceneNativeBridge.DestroyEntity(world.NativeHandle, childToRemove);
                    }
                    break;
            }
        }
    }

    /// <summary>
    /// 将实例的所有覆盖还原为 Prefab 资产中的原始状态。
    /// 支持全部 5 种覆盖类型的还原。
    /// </summary>
    /// <param name="instance">Prefab 实例。</param>
    /// <param name="world">场景世界。</param>
    public static void RevertToPrefab(PrefabInstance instance, SceneWorld world)
    {
        ArgumentNullException.ThrowIfNull(instance);
        ArgumentNullException.ThrowIfNull(world);

        var asset = instance.Source;

        foreach (var ovr in instance.Overrides)
        {
            var handle = instance.GetEntity(ovr.EntityLocalIndex);
            if (handle.Value == 0)
            {
                continue;
            }

            // 找到资产中的原始实体数据
            var entityData = asset.Entities.Find(e => e.LocalIndex == ovr.EntityLocalIndex);

            switch (ovr.Type)
            {
                case PrefabOverrideType.PropertyModified:
                    // 还原为资产中的原始组件数据
                    if (entityData != null &&
                        entityData.Components.TryGetValue(ovr.ComponentTypeId, out var originalData))
                    {
                        SceneNativeBridge.SetComponentData(
                            world.NativeHandle, handle, ovr.ComponentTypeId, originalData);
                    }
                    break;

                case PrefabOverrideType.ComponentAdded:
                    // 还原：移除覆盖时添加的组件
                    SceneNativeBridge.RemoveComponent(world.NativeHandle, handle, ovr.ComponentTypeId);
                    break;

                case PrefabOverrideType.ComponentRemoved:
                    // 还原：从资产数据重新添加被移除的组件
                    if (entityData != null &&
                        entityData.Components.TryGetValue(ovr.ComponentTypeId, out var removedData))
                    {
                        SceneNativeBridge.AddComponent(world.NativeHandle, handle, ovr.ComponentTypeId);
                        SceneNativeBridge.SetComponentData(
                            world.NativeHandle, handle, ovr.ComponentTypeId, removedData);
                    }
                    break;

                case PrefabOverrideType.ChildAdded:
                    // 还原：销毁覆盖时添加的子实体
                    // ChildAdded 的子实体在 InstanceMap 中以额外负索引存储
                    // 此处通过 ComponentTypeId 编码的子局部索引定位
                    var addedChild = instance.GetEntity((int)ovr.ComponentTypeId);
                    if (addedChild.Value != 0)
                    {
                        SceneNativeBridge.DestroyEntity(world.NativeHandle, addedChild);
                    }
                    break;

                case PrefabOverrideType.ChildRemoved:
                    // 还原：重新创建被移除的子实体
                    // 从资产中查找该子实体的原始数据
                    if (entityData != null)
                    {
                        var restoredChild = SceneNativeBridge.CreateEntity(world.NativeHandle);
                        if (restoredChild.Value != 0)
                        {
                            // 还原组件数据
                            foreach (var (typeId, data) in entityData.Components)
                            {
                                SceneNativeBridge.SetComponentData(
                                    world.NativeHandle, restoredChild, typeId, data);
                            }
                            // 恢复层级
                            SceneNativeBridge.SetParent(world.NativeHandle, restoredChild, handle);
                        }
                    }
                    break;
            }
        }

        instance.ClearOverrides();
    }

    /// <summary>
    /// 检测实例与源 Prefab 之间的差异，生成覆盖列表。
    /// 比较每个实体的组件数据，发现不一致时记录 PropertyModified 覆盖。
    /// </summary>
    /// <param name="world">场景世界。</param>
    /// <param name="instance">Prefab 实例。</param>
    /// <returns>检测到的差异覆盖列表。</returns>
    public static List<PrefabOverride> DetectDifferences(SceneWorld world, PrefabInstance instance)
    {
        ArgumentNullException.ThrowIfNull(world);
        ArgumentNullException.ThrowIfNull(instance);

        var differences = new List<PrefabOverride>();
        var asset = instance.Source;

        foreach (var entityData in asset.Entities)
        {
            var handle = instance.GetEntity(entityData.LocalIndex);
            if (handle.Value == 0)
            {
                continue;
            }

            foreach (var (typeId, originalData) in entityData.Components)
            {
                // 读取当前运行时组件数据
                var currentData = ReadComponentRaw(world.NativeHandle, handle, typeId, originalData.Length);
                if (currentData == null)
                {
                    continue;
                }

                // 逐字节比较
                if (!BytesEqual(originalData, currentData))
                {
                    differences.Add(new PrefabOverride
                    {
                        Type = PrefabOverrideType.PropertyModified,
                        EntityLocalIndex = entityData.LocalIndex,
                        ComponentTypeId = typeId,
                        NewValue = currentData,
                    });
                }
            }
        }

        return differences;
    }

    /// <summary>
    /// 添加组件到已有实体（AddComponent 封装，供外部按 TypeId 操作）。
    /// </summary>
    internal static NNSceneResult AddComponent(
        ulong sceneHandle, NNEntityHandle entity, ulong componentTypeId)
    {
        if (entity.Value == 0 || sceneHandle == 0)
        {
            return NNSceneResult.Invalid;
        }

        return SceneNativeBridge.AddComponent(sceneHandle, entity, componentTypeId);
    }

    /// <summary>
    /// 移除组件（RemoveComponent 封装）。
    /// </summary>
    internal static NNSceneResult RemoveComponent(
        ulong sceneHandle, NNEntityHandle entity, ulong componentTypeId)
    {
        if (entity.Value == 0 || sceneHandle == 0)
        {
            return NNSceneResult.Invalid;
        }

        return SceneNativeBridge.RemoveComponent(sceneHandle, entity, componentTypeId);
    }

    private static unsafe byte[]? ReadComponentRaw(ulong sceneHandle, NNEntityHandle entity, ulong typeId, int expectedSize)
    {
        if (!SceneNativeBridge.TryGetSceneApi(out var api) || api.GetComponent == null)
        {
            return null;
        }

        var buffer = new byte[expectedSize];
        fixed (byte* p = buffer)
        {
            var result = api.GetComponent(sceneHandle, entity.Value, typeId, p, (uint)expectedSize);
            return result == NNSceneResult.Ok ? buffer : null;
        }
    }

    private static bool BytesEqual(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b)
    {
        return a.SequenceEqual(b);
    }

    private static void RollbackCreatedEntities(SceneWorld world, Dictionary<int, NNEntityHandle> created)
    {
        foreach (var handle in created.Values)
        {
            if (handle.Value != 0)
            {
                SceneNativeBridge.DestroyEntity(world.NativeHandle, handle);
            }
        }
    }
}
