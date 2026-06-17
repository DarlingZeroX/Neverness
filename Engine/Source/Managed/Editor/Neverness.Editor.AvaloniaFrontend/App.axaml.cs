using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Controls.Templates;
using Avalonia.Data;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Neverness.Editor.AvaloniaFrontend.Controls;
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
    internal static void ConfigureDockControl(global::Dock.Avalonia.Controls.DockControl dockControl, global::Dock.Model.Core.IFactory? factory, global::Dock.Model.Core.IDock? layout)
    {
        dockControl.Factory = factory ?? AvaloniaFrontendModule.DockFactory;
        dockControl.HostWindowFactory = CreateDockHostWindow;
        dockControl.AutoCreateDataTemplates = true;

        // DockControl 本地 DataTemplates 优先级高于 Application.DataTemplates。
        // 浮动窗口内部新建的 DockControl 需要在本地重新覆盖 Document/Tool 模板，
        // 否则会落回 Dock 自动生成的默认模板，导致 Context 中的实际控件不显示。
        dockControl.DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Document),
            (_, _) =>
            {
                var host = new DockContentHost();
                host.Bind(DockContentHost.HostedContentProperty, new Binding(nameof(global::Dock.Model.Mvvm.Core.DockableBase.Context)));
                return host;
            }));

        dockControl.DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Tool),
            (_, _) =>
            {
                var host = new DockContentHost();
                host.Bind(DockContentHost.HostedContentProperty, new Binding(nameof(global::Dock.Model.Mvvm.Core.DockableBase.Context)));
                return host;
            }));

        dockControl.Layout = layout;
    }

    internal static global::Dock.Avalonia.Controls.HostWindow CreateDockHostWindow()
    {
        return new global::Dock.Avalonia.Controls.HostWindow
        {
            IsToolWindow = true,
            ToolChromeControlsWholeWindow = true,
            Background = new global::Avalonia.Media.SolidColorBrush(global::Avalonia.Media.Color.Parse("#FF1E1E1E")),
        };
    }

    internal static global::Dock.Avalonia.Controls.DockControl CreateFloatingDockControl(global::Dock.Model.Core.IFactory? factory, global::Dock.Model.Core.IDock? layout)
    {
        var dockControl = new global::Dock.Avalonia.Controls.DockControl();
        ConfigureDockControl(dockControl, factory, layout);
        return dockControl;
    }

    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        // Dock 主题已在 App.axaml 中声明（fluent:DockFluentTheme）
        RequestedThemeVariant = ThemeVariant.Dark;

        // 全局 DataTemplate——注册在 Application 级别，所有窗口（含浮动窗口）共享

        // DockWindow 模板：原生浮动窗口 Content 实际是 DockWindow，需取其 Layout 创建 DockControl。
        DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Core.DockWindow),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Core.DockWindow dockWindow)
                {
                    return CreateFloatingDockControl(dockWindow.Factory, dockWindow.Layout as global::Dock.Model.Core.IDock);
                }

                return new TextBlock { Text = "No Content" };
            }));

        // RootDock 模板：某些场景 Content 直接是 RootDock 模型，也需要 DockControl 渲染。
        DataTemplates.Insert(0, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.RootDock),
            (data, _) =>
            {
                if (data is global::Dock.Model.Mvvm.Controls.RootDock rootDock)
                {
                    return CreateFloatingDockControl(AvaloniaFrontendModule.DockFactory, rootDock);
                }
                return new TextBlock { Text = "No Content" };
            }));

        DataTemplates.Insert(1, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Document),
            (_, _) =>
            {
                var host = new DockContentHost();
                host.Bind(DockContentHost.HostedContentProperty, new Binding(nameof(global::Dock.Model.Mvvm.Core.DockableBase.Context)));
                return host;
            }));

        DataTemplates.Insert(1, new FuncDataTemplate(
            typeof(global::Dock.Model.Mvvm.Controls.Tool),
            (_, _) =>
            {
                var host = new DockContentHost();
                host.Bind(DockContentHost.HostedContentProperty, new Binding(nameof(global::Dock.Model.Mvvm.Core.DockableBase.Context)));
                return host;
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
