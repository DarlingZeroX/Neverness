namespace Neverness.Runtime.Scene;

/// <summary>
/// 实体抽象接口——与 ECS 实现无关。
/// 提供组件和标签的增删查改操作。
/// </summary>
public interface IEntity
{
    /// <summary>实体 ID（唯一标识）。</summary>
    int Id { get; }

    /// <summary>实体名称（可选，用于调试和显示）。</summary>
    string? Name { get; set; }

    /// <summary>是否有效（未被销毁）。</summary>
    bool IsValid { get; }

    // ── 组件操作 ──

    /// <summary>添加组件。如果已存在则覆盖。</summary>
    void Add<T>(T component) where T : struct, IComponent;

    /// <summary>移除组件。如果不存在则忽略。</summary>
    void Remove<T>() where T : struct, IComponent;

    /// <summary>获取组件引用。如果不存在则抛异常。</summary>
    ref T Get<T>() where T : struct, IComponent;

    /// <summary>检查是否拥有组件。</summary>
    bool Has<T>() where T : struct, IComponent;

    /// <summary>尝试获取组件。如果不存在返回 false。</summary>
    bool TryGet<T>(out T component) where T : struct, IComponent;

    // ── 标签操作 ──

    /// <summary>添加标签。</summary>
    void AddTag<T>() where T : struct, ITag;

    /// <summary>移除标签。</summary>
    void RemoveTag<T>() where T : struct, ITag;

    /// <summary>检查标签。</summary>
    bool HasTag<T>() where T : struct, ITag;

    // ── 生命周期 ──

    /// <summary>销毁实体。</summary>
    void Destroy();
}
