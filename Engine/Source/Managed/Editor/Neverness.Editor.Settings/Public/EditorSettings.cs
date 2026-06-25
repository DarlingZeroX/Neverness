using Neverness.Editor.Settings.Private.Descriptors;

namespace Neverness.Editor.Settings;

/// <summary>
/// 编辑器偏好设置的全局静态访问器。
/// 模式与 RuntimeSettings 一致：声明时创建实例，Install 时加载 JSON。
///
/// 用法：
/// <code>
/// // 读取
/// var ide = EditorSettings.Preferences.PreferredIDE;
/// int size = EditorSettings.Preferences.FontSize;
///
/// // 写入（触发 PropertyChanged + SettingsChanged）
/// EditorSettings.Preferences.FontSize = 16;
///
/// // 监听变更
/// EditorSettings.SettingsChanged += (tableId, field) => { ... };
/// </code>
/// </summary>
public static class EditorSettings
{
    // ── 设置表实例 ──

    /// <summary>编辑器偏好设置表（全局单例）。</summary>
    public static EditorPreferencesSettings Preferences { get; } = new();

    /// <summary>编辑器会话状态表（全局单例，Scope = Project）。</summary>
    public static EditorSessionSettings Session { get; } = new();

    // ── 变更通知 ──

    /// <summary>
    /// 编辑器偏好设置变更事件（tableId, fieldName）。
    /// 由 SettingsModule.Install() 订阅 PropertyChanged 后触发。
    /// </summary>
    public static event Action<string, string>? SettingsChanged;

    // ── 内部方法（SettingsModule 调用） ──

    /// <summary>触发设置变更事件。</summary>
    internal static void RaiseSettingChanged(string tableId, string fieldName)
    {
        SettingsChanged?.Invoke(tableId, fieldName);
    }
}
