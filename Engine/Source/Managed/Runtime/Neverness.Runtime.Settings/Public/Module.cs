namespace Neverness.Runtime.Settings;

/// <summary>
/// Neverness.Runtime.Settings 模块入口。
///
/// 职责：
///   - 初始化运行时设置系统（通过 VFS 加载 JSON 配置文件）
///   - 关闭时持久化所有设置
///
/// 用法：
/// <code>
/// // VFS 就绪后
/// SettingsModule.Initialize();
///
/// // 退出时
/// SettingsModule.Shutdown();
/// </code>
/// </summary>
public static class SettingsModule
{
    private static bool _initialized;

    /// <summary>模块是否已初始化。</summary>
    public static bool IsInitialized => _initialized;

    /// <summary>
    /// 初始化设置模块——通过 VFS 加载 JSON 配置文件。
    /// 应在 VFS 就绪后调用一次。
    /// </summary>
    public static void Initialize()
    {
        if (_initialized)
        {
            Console.WriteLine("[SettingsModule] 已初始化，跳过重复调用。");
            return;
        }

        RuntimeSettings.Initialize();
        _initialized = true;
    }

    /// <summary>
    /// 关闭设置模块——持久化所有设置到磁盘。
    /// 应在应用退出时调用。
    /// </summary>
    public static void Shutdown()
    {
        if (!_initialized)
            return;

        RuntimeSettings.SaveAll();
        _initialized = false;

        Console.WriteLine("[SettingsModule] 已关闭，设置已保存。");
    }
}
