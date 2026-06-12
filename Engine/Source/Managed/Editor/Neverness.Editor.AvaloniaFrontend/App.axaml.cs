using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Avalonia.Themes.Fluent;
using Dock.Avalonia.Themes.Fluent;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.AvaloniaFrontend.Views;

namespace Neverness.Editor.AvaloniaFrontend;

/// <summary>
/// Avalonia 应用入口——编辑器 UI 框架初始化。
///
/// 关键：必须添加 DockFluentTheme，否则 DockControl 不会渲染任何内容。
/// </summary>
public class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        // 添加 Dock 主题（必须，否则 DockControl 不渲染）
        Styles.Add(new DockFluentTheme());
        RequestedThemeVariant = ThemeVariant.Dark;

        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            // 在 Avalonia UI 线程中创建 MainEditorWindow
            var mainWindow = new MainEditorWindow();
            desktop.MainWindow = mainWindow;

            // 保存到模块静态字段
            AvaloniaFrontendModule.MainWindow = mainWindow;

            // 通知 InstallShell() 窗口已就绪
            AvaloniaFrontendModule.NotifyWindowReady();
        }

        base.OnFrameworkInitializationCompleted();
    }
}
