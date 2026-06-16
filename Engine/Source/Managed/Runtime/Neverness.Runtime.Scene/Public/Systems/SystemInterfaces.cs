namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景系统接口——所有场景系统的根接口。
/// 兼容旧的 ISceneSystem 和新的 ISceneSystem。
/// </summary>
public interface ISceneSystem : IDisposable
{
    /// <summary>系统名称（用于调试和日志）。</summary>
    string Name { get; }

    /// <summary>系统优先级（越小越先执行，默认 0）。</summary>
    int Priority => 0;

    /// <summary>系统标签掩码（Editor PlayMode 过滤用）。</summary>
    SceneSystemTags Tags => SceneSystemTags.All;

    /// <summary>是否已初始化。</summary>
    bool IsInitialized { get; }

    /// <summary>初始化系统（World 已就绪时调用）。</summary>
    void Initialize(IScene scene);

    /// <summary>每帧更新。</summary>
    void Update(float deltaTime);

    /// <summary>固定步长更新（仅 FixedUpdate 组的系统需要实现）。</summary>
    void FixedUpdate(float fixedDeltaTime) { }

    /// <summary>关闭系统（逆序调用）。</summary>
    void Shutdown();
}

/// <summary>
/// 旧的系统初始化接口（兼容旧代码）。
/// </summary>
[Obsolete("使用 ISceneSystem.Initialize(IScene) 替代")]
public interface ISystemInitialize : ISceneSystem
{
    void OnInitialize(SceneWorld world);
}

/// <summary>
/// 旧的系统 Tick 接口（兼容旧代码）。
/// </summary>
[Obsolete("使用 ISceneSystem.Update(float) 替代")]
public interface ISystemTick : ISceneSystem
{
    TickGroup TickGroup { get; }
    void OnTick(float deltaTime);
}

/// <summary>
/// 旧的固定步长 Tick 接口（兼容旧代码）。
/// </summary>
[Obsolete("使用 ISceneSystem.FixedUpdate(float) 替代")]
public interface ISystemFixedTick : ISceneSystem
{
    void OnFixedTick(float fixedDeltaTime);
}

/// <summary>
/// 旧的 LateTick 接口（兼容旧代码）。
/// </summary>
[Obsolete("使用 ISceneSystem.Update(float) 替代")]
public interface ISystemLateTick : ISceneSystem
{
    void OnLateTick(float deltaTime);
}

/// <summary>
/// 旧的系统关闭接口（兼容旧代码）。
/// </summary>
[Obsolete("使用 ISceneSystem.Shutdown() 替代")]
public interface ISystemShutdown : ISceneSystem
{
    void OnShutdown();
}

/// <summary>
/// 场景系统标签——标记系统的执行类别。
/// Runtime 侧仅提供标签机制，不知道 PlayMode 的存在。
/// Editor 侧决定每个模式用什么 tagMask。
/// </summary>
[Flags]
public enum SceneSystemTags : ushort
{
    None = 0,

    /// <summary>编辑器系统（Gizmo、选择高亮、层级同步）。</summary>
    Editor = 1 << 0,

    /// <summary>游戏逻辑系统（角色控制器、AI、游戏状态机）。</summary>
    Gameplay = 1 << 1,

    /// <summary>渲染系统（Render、PostProcess）。</summary>
    Render = 1 << 2,

    /// <summary>音频系统（Audio、Music）。</summary>
    Audio = 1 << 3,

    /// <summary>物理系统（Physics、Collision）。</summary>
    Physics = 1 << 4,

    /// <summary>流式加载系统（Streaming、LOD）。</summary>
    Streaming = 1 << 5,

    /// <summary>所有标签。</summary>
    All = ushort.MaxValue,
}

/// <summary>
/// SceneSystemTags 默认值常量。
/// 未标记 [SceneSystemTag] 的系统默认为此值。
/// </summary>
internal static class SceneSystemTagDefaults
{
    /// <summary>
    /// 默认标签：Render | Audio | Streaming。
    /// 所有模式都应 tick 的基础设施系统。
    /// </summary>
    public const SceneSystemTags Default =
        SceneSystemTags.Render |
        SceneSystemTags.Audio |
        SceneSystemTags.Streaming;
}

/// <summary>
/// 标记 ECS 系统的标签。
/// 未标记的系统默认为 <see cref="SceneSystemTagDefaults.Default"/>。
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class SceneSystemTagAttribute(SceneSystemTags tags) : Attribute
{
    public SceneSystemTags Tags { get; } = tags;
}
