using System.Numerics;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene.Systems;

/// <summary>
/// 变换系统——DFS 遍历层级树，计算世界矩阵。
/// 替代 C++ NNTransformSystem。
/// 使用 System.Numerics 替代 GLM。
/// </summary>
public sealed class TransformSystem : ISceneSystem
{
    private IScene? _scene;

    public string Name => "TransformSystem";
    public int Priority => 100;
    public SceneSystemTags Tags => SceneSystemTags.All;
    public bool IsInitialized { get; private set; }

    public void Initialize(IScene scene)
    {
        _scene = scene;
        IsInitialized = true;
    }

    public void Update(float deltaTime)
    {
        if (_scene == null) return;

        // 收集所有需要更新的实体
        var entities = new List<IEntity>();
        _scene.Query<TransformComponent>().ForEach((ref TransformComponent transform, IEntity entity) =>
        {
            entities.Add(entity);
        });

        // 按层级深度排序（根节点在前，子节点在后）
        entities.Sort((a, b) =>
        {
            var depthA = GetHierarchyDepth(a);
            var depthB = GetHierarchyDepth(b);
            return depthA.CompareTo(depthB);
        });

        // 按排序顺序更新世界矩阵
        foreach (var entity in entities)
        {
            ref var transform = ref entity.Get<TransformComponent>();

            // 计算局部矩阵
            var localMatrix = transform.ComputeLocalMatrix();

            // 获取父实体的世界矩阵
            var parent = _scene.GetParent(entity);
            if (parent != null && parent.IsValid && parent.Has<TransformComponent>())
            {
                ref var parentTransform = ref parent.Get<TransformComponent>();
                transform.WorldMatrix = localMatrix * parentTransform.WorldMatrix;
            }
            else
            {
                // 根节点，世界矩阵 = 局部矩阵
                transform.WorldMatrix = localMatrix;
            }
        }
    }

    /// <summary>获取实体的层级深度。</summary>
    private int GetHierarchyDepth(IEntity entity)
    {
        int depth = 0;
        var current = entity;
        while (current != null && current.IsValid)
        {
            var parent = _scene?.GetParent(current);
            if (parent == null || !parent.IsValid) break;
            current = parent;
            depth++;
        }
        return depth;
    }

    public void FixedUpdate(float fixedDeltaTime) { }

    public void Shutdown()
    {
        _scene = null;
        IsInitialized = false;
    }

    public void Dispose() { }
}
