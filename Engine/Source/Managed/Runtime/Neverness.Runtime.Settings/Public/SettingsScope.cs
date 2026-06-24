namespace Neverness.Runtime.Settings;

/// <summary>
/// 设置范围——区分项目设置和用户偏好。
///
/// Project：项目级设置，随项目存储，影响游戏运行时行为（如图形、音频）。
/// User：用户级偏好，存储在用户目录，影响编辑器体验（如主题、快捷键）。
/// </summary>
public enum SettingsScope
{
    /// <summary>项目设置——随项目存储，团队共享。</summary>
    Project,

    /// <summary>用户偏好——存储在用户目录，个人专属。</summary>
    User
}
