using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 资产——描述可实例化的实体子图。
/// 可从文件加载（经 VFS）或运行时构建。
/// </summary>
public sealed class PrefabAsset
{
    /// <summary>资产 GUID。</summary>
    public NNGuid Guid { get; init; }

    /// <summary>Prefab 名称。</summary>
    public string Name { get; init; } = string.Empty;

    /// <summary>原始实体描述列表（有序，根实体在 index 0）。</summary>
    public List<PrefabEntityData> Entities { get; } = [];

    /// <summary>从运行时世界中选择的实体构建 Prefab 资产。</summary>
    /// <param name="world">源世界。</param>
    /// <param name="rootEntity">根实体句柄。</param>
    /// <returns>构建的 Prefab 资产。</returns>
    public static PrefabAsset FromEntity(SceneWorld world, NNEntityHandle rootEntity)
    {
        ArgumentNullException.ThrowIfNull(world);

        var asset = new PrefabAsset { Name = "Prefab" };
        var handleToLocal = new Dictionary<ulong, int>();
        var index = 0;

        // 递归收集实体（BFS）
        var queue = new Queue<NNEntityHandle>();
        queue.Enqueue(rootEntity);

        while (queue.Count > 0)
        {
            var handle = queue.Dequeue();
            if (handle.Value == 0 || handleToLocal.ContainsKey(handle.Value))
            {
                continue;
            }

            var entity = world.Entities.Find(handle);
            if (entity == null)
            {
                continue;
            }

            var localIndex = index++;
            handleToLocal[handle.Value] = localIndex;

            var data = new PrefabEntityData
            {
                LocalIndex = localIndex,
                DisplayName = entity.DisplayName,
            };

            // 父索引
            var parentHandle = SceneNativeBridge.GetParent(world.NativeHandle, handle);
            if (parentHandle.Value != 0 && handleToLocal.TryGetValue(parentHandle.Value, out var parentIdx))
            {
                data = new PrefabEntityData
                {
                    LocalIndex = localIndex,
                    ParentIndex = parentIdx,
                    DisplayName = data.DisplayName,
                };
            }

            // 遍历子实体入队
            // 注意：当前无 GetChildren API，嵌套 Prefab 从实体集合中推断
            // 后续可通过 Native GetChildren 批量获取

            asset.Entities.Add(data);
        }

        return asset;
    }
}
