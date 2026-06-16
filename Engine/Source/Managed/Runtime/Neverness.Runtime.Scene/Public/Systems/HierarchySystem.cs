using Neverness.Runtime.Scene.Components;

namespace Neverness.Runtime.Scene.Systems;

/// <summary>
/// 层级系统——管理父子关系、循环检测、深度计算。
/// 替代 C++ NNHierarchySystem。
/// </summary>
public sealed class HierarchySystem : ISceneSystem
{
    private IScene? _scene;

    // ── ISceneSystem 属性 ──

    public string Name => "HierarchySystem";
    public int Priority => 0; // 最先执行
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

        // 同步 ChildCount 和 Depth 到组件
        _scene.Query<RelationshipComponent>().ForEach((ref RelationshipComponent rel, IEntity entity) =>
        {
            // 更新 ChildCount
            rel.ChildCount = (uint)_scene.GetChildren(entity).Count;

            // 更新 Depth（从父链计算）
            rel.Depth = ComputeDepth(entity);
        });
    }

    public void FixedUpdate(float fixedDeltaTime) { }

    public void Shutdown()
    {
        _scene = null;
        IsInitialized = false;
    }

    public void Dispose() { }

    // ── 层级操作 API ──

    /// <summary>计算实体的层级深度。</summary>
    private uint ComputeDepth(IEntity entity)
    {
        if (_scene == null) return 0;

        uint depth = 0;
        var current = entity;

        while (current != null && current.IsValid)
        {
            var parent = _scene.GetParent(current);
            if (parent == null || !parent.IsValid) break;
            current = parent;
            depth++;

            // 安全阀：防止极端情况下的无限循环
            if (depth > 1000)
            {
                System.Diagnostics.Debug.WriteLine("[HierarchySystem] 层级深度超过 1000，可能存在循环");
                break;
            }
        }

        return depth;
    }
}
