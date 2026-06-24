using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Settings.Private;
using Neverness.Editor.Settings.Private.Descriptors;
using Neverness.Runtime.Settings.Descriptors;

namespace Neverness.Editor.Settings;

/// <summary>
/// 设置系统模块入口——由 EditorApplicationRunner 调用 Install()。
///
/// 负责：
/// 1. 注册内置设置表（Graphics / Audio / EditorPreferences）
/// 2. 加载已保存的设置
/// 3. 注册 ISettingsService 到服务定位器
/// 4. 注册菜单命令和菜单项
/// </summary>
public static class SettingsModule
{
    /// <summary>安装设置系统模块。</summary>
    public static void Install()
    {
        var registry = new SettingsRegistry();
        var storage = new SettingsStorage();
        var service = new SettingsServiceImpl(registry, storage);

        // 注册内置设置表
        RegisterBuiltinSettings(service);

        // 加载已保存的设置
        service.LoadAll();

        // 注册到服务定位器
        CoreModuleImp.Context.RegisterService<ISettingsService>(service);

        // 注册菜单命令和菜单项
        RegisterMenuIntegration(service);

        Console.WriteLine("[SettingsModule] 设置系统已初始化。");
    }

    /// <summary>注册内置设置表。</summary>
    private static void RegisterBuiltinSettings(SettingsServiceImpl service)
    {
        service.Register(
            new GraphicsSettings(),
            GraphicsSettingsDescriptor.Instance);

        service.Register(
            new AudioSettings(),
            AudioSettingsDescriptor.Instance);

        service.Register(
            new EditorPreferencesSettings(),
            EditorPreferencesSettingsDescriptor.Instance);
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
