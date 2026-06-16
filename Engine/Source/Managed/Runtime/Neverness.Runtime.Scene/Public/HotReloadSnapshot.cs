namespace Neverness.Runtime.Scene;

/// <summary>
/// 单个场景世界的热重载快照。
/// 在程序集卸载前保存，新程序集加载后恢复。
/// </summary>
public sealed class HotReloadSnapshot
{
    /// <summary>场景名称。</summary>
    public string Name { get; set; } = "";

    /// <summary>实体 ID 列表。</summary>
    public List<int> EntityIds { get; set; } = new();

    /// <summary>捕获时间。</summary>
    public DateTimeOffset CapturedAt { get; set; }
}

/// <summary>
/// 全局热重载快照——包含所有已加载世界。
/// </summary>
public sealed class GlobalHotReloadSnapshot
{
    /// <summary>世界快照映射（名称 → 快照）。</summary>
    public Dictionary<string, HotReloadSnapshot> Worlds { get; set; } = new();

    /// <summary>激活世界名称。</summary>
    public string? ActiveWorldName { get; set; }

    /// <summary>捕获时间。</summary>
    public DateTimeOffset CapturedAt { get; set; }
}
