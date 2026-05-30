namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景系统标签——标记系统的执行类别。
/// Runtime 侧仅提供标签机制，不知道 PlayMode 的存在。
/// Editor 侧决定每个模式用什么 tagMask。
///
/// 只定义原子标签，不定义组合标签。
/// 组合逻辑由 Editor 侧 PlayModeController 负责。
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
