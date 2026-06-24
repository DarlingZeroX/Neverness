using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Settings.Private;
using Neverness.Editor.Settings.Private.Descriptors;
using Neverness.Runtime.Settings;
using Neverness.Runtime.Settings.Descriptors;
using Neverness.Runtime.VFS;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Settings;

/// <summary>
/// 设置系统模块入口——由 EditorApplicationRunner 调用 Install()。
///
/// 负责：
/// 1. 初始化 RuntimeSettings（全局静态访问）
/// 2. 注册所有设置表到 Editor 服务（供 UI 编辑）
/// 3. 加载已保存的 Editor 专属设置
/// 4. 注册 ISettingsService 到服务定位器
/// 5. 注册菜单命令和菜单项
/// </summary>
public static class SettingsModule
{
    /// <summary>安装设置系统模块。</summary>
    public static void Install()
    {
        // 1. 初始化 RuntimeSettings（全局静态访问，从项目目录加载）
        var settingsDir = VFSService.GetAbsolutePath(ProjectPaths.Settings.FullPath);
        if (string.IsNullOrEmpty(settingsDir))
        {
            Console.WriteLine("[SettingsModule] 无法获取设置目录路径，RuntimeSettings 初始化跳过。");
            return;
        }

        var runtimeSettingsDir = Path.Combine(settingsDir, "Settings");
        RuntimeSettings.Initialize(runtimeSettingsDir);

        // 2. 创建 Editor 服务
        var registry = new SettingsRegistry();
        var storage = new SettingsStorage();
        var service = new SettingsServiceImpl(registry, storage);

        // 3. 注册 Runtime 设置表（使用 RuntimeSettings 的单例实例，保证单一数据源）
        service.Register(RuntimeSettings.Graphics, GraphicsSettingsDescriptor.Instance);
        service.Register(RuntimeSettings.Audio, AudioSettingsDescriptor.Instance);

        // 4. 注册 Editor 专属设置表
        service.Register(
            new EditorPreferencesSettings(),
            EditorPreferencesSettingsDescriptor.Instance);

        // 5. 加载 Editor 专属设置（Runtime 设置已由 RuntimeSettings.Initialize 加载）
        service.Load("editorPreferences");

        // 6. 转发 RuntimeSettings.SettingsChanged 到 ISettingsService.SettingChanged
        RuntimeSettings.SettingsChanged += (tableId, field) =>
            service.RaiseSettingChanged(tableId, field);

        // 7. 注册到服务定位器
        CoreModuleImp.Context.RegisterService<ISettingsService>(service);

        // 8. 注册菜单命令和菜单项
        RegisterMenuIntegration(service);

        Console.WriteLine("[SettingsModule] 设置系统已初始化。");
    }

    /// <summary>注册菜单命令和菜单项。</summary>
    private static void RegisterMenuIntegration(SettingsServiceImpl service)
    {
        // 注册 "editor.project_settings" 命令（Project Settings）
        var projectSettingsCmd = new EditorCommand
        {
            Id = "editor.project_settings",
            DisplayName = "Project Settings",
            Execute = _ => service.ShowProjectSettingsWindow(),
            Tooltip = "打开项目设置（图形、音频等运行时配置）"
        };
        CoreModuleImp.Context.Menus.RegisterCommand(projectSettingsCmd);

        // 注册 "editor.preferences" 命令（Preferences）
        var preferencesCmd = new EditorCommand
        {
            Id = "editor.preferences",
            DisplayName = "Preferences",
            Execute = _ => service.ShowPreferencesWindow(),
            Tooltip = "打开编辑器偏好设置（主题、快捷键等）"
        };
        CoreModuleImp.Context.Menus.RegisterCommand(preferencesCmd);

        // 注册菜单项：Edit/Project Settings...
        CoreModuleImp.Context.Menus.Register(new EditorMenuItem(
            "Edit/Project Settings...",
            CommandId: "editor.project_settings",
            SortOrder: 800,
            Icon: "⚙️",
            Tooltip: "打开项目设置（图形、音频等运行时配置）"));

        // 注册菜单项：Edit/Preferences...
        CoreModuleImp.Context.Menus.Register(new EditorMenuItem(
            "Edit/Preferences...",
            CommandId: "editor.preferences",
            SortOrder: 900,
            Icon: "🔧",
            Tooltip: "打开编辑器偏好设置"));

        Console.WriteLine("[SettingsModule] 菜单集成已注册 (editor.project_settings, editor.preferences)。");
    }
}
