using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 热重载状态快照——保存 Managed 侧的运行时状态，
/// 使程序集重载后能恢复到重载前的运行状态。
///
/// Native 场景数据（entt::registry）不受 C# 程序集重载影响，
/// 因此只需保存 Managed 侧的映射和配置信息。
/// </summary>
public sealed class HotReloadSnapshot
{
    /// <summary>世界名称。</summary>
    public string Name { get; init; } = string.Empty;

    /// <summary>Native 场景句柄（程序集重载后仍然有效）。</summary>
    public ulong NativeHandle { get; init; }

    /// <summary>场景资产 GUID（如有）。</summary>
    public NNGuid AssetGuid { get; init; }

    /// <summary>场景资产 VFS 路径（null = 未保存的新场景）。</summary>
    public string? AssetPath { get; init; }

    /// <summary>所有存活实体的 Native 句柄值（用于重建 EntityRegistry）。</summary>
    public IReadOnlyList<ulong> EntityHandles { get; init; } = [];

    /// <summary>捕获时的时间戳（UTC）。</summary>
    public DateTimeOffset CapturedAt { get; init; }
}

/// <summary>
/// 全局热重载快照——包含所有已加载世界的快照。
/// 用于 SceneManager 级别的热重载协调。
/// </summary>
public sealed class GlobalHotReloadSnapshot
{
    /// <summary>各世界的快照（名称 → 快照）。</summary>
    public IReadOnlyDictionary<string, HotReloadSnapshot> Worlds { get; init; } =
        new Dictionary<string, HotReloadSnapshot>();

    /// <summary>激活世界名称（如有）。</summary>
    public string? ActiveWorldName { get; init; }

    /// <summary>捕获时的时间戳（UTC）。</summary>
    public DateTimeOffset CapturedAt { get; init; }
}
