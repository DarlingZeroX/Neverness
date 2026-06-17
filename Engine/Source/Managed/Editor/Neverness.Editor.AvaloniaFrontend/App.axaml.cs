using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Controls.Templates;
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

        // 全局 DataTemplate——注册在 Application 级别，所有窗口（含浮动窗口）共享
        // 浮动窗口的 HostWindow.Content 是 Dock 模型对象（RootDock），需要 DataTemplate 渲染

        // RootDock 模板：创建 DockControl 渲染浮动布局
        // 浮动窗口的 Content 是 RootDock 模型，需要 DockControl 来渲染
        DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.RootDock),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.RootDock rootDock)
                {
                    return new global::Dock.Avalonia.Controls.DockControl
                    {
                        Factory = AvaloniaFrontendModule.DockFactory,
                        Layout = rootDock,
                    };
                }
                return new TextBlock { Text = "No Content" };
            }));

        DataTemplates.Insert(1, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Document),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.Document doc && doc.Context is Control ctrl)
                    return ctrl;
                return new TextBlock { Text = "No Content" };
            }));

        DataTemplates.Insert(2, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Tool),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.Tool tool && tool.Context is Control ctrl)
                    return ctrl;
                return new TextBlock { Text = "No Content" };
            }));

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
