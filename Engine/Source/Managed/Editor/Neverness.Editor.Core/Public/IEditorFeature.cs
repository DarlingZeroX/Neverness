namespace Neverness.Editor.Core.Public;

/// <summary>
/// 所有 Feature 模块实现此接口，向 Core 注册自身。
/// Framework 不感知具体 Feature；Core 通过此接口统一管理生命周期。
/// </summary>
public interface IEditorFeature
{
    /// <summary>Feature 唯一标识（如 "com.neverness.assets"）。</summary>
    string FeatureId { get; }

    /// <summary>Feature 显示名称。</summary>
    string DisplayName { get; }

    /// <summary>依赖的 Feature 列表（用于拓扑排序初始化）。</summary>
    IReadOnlyList<string> Dependencies { get; }

    /// <summary>Feature 初始化：注册面板、菜单贡献者、服务等。</summary>
    void Initialize(IEditorContext context);

    /// <summary>Feature 关闭：注销面板、释放资源。</summary>
    void Shutdown(IEditorContext context);
}
