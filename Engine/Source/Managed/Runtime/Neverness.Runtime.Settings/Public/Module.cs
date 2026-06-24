namespace Neverness.Runtime.Settings;

/// <summary>
/// Neverness.Runtime.Settings 模块入口。
///
/// 职责：
///   - 初始化运行时设置系统（加载 JSON 配置文件）
///   - 关闭时持久化所有设置
///
/// 用法：
/// <code>
/// // 启动时
/// SettingsModule.Initialize("path/to/settings");
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
    /// 初始化设置模块——从指定目录加载 JSON 配置文件。
    /// 应在应用启动时调用一次。
    /// </summary>
    /// <param name="settingsDir">设置文件目录路径。</param>
    public static void Initialize(string settingsDir)
    {
        if (_initialized)
        {
            Console.WriteLine("[SettingsModule] 已初始化，跳过重复调用。");
            return;
        }

        RuntimeSettings.Initialize(settingsDir);
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
