using Avalonia.Threading;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Framework.Public.Services;
using Neverness.Editor.AvaloniaFrontend.Services;
using Neverness.Editor.AvaloniaFrontend.Views;

namespace Neverness.Editor.AvaloniaFrontend.Public;

/// <summary>
/// AvaloniaFrontend 模块入口——注册 Avalonia 渲染器实现到 Framework 委托层。
///
/// 参考 ImGuiFrontendModule 的结构，在 NevernessEditor 启动时调用。
/// 将 Avalonia 特定的渲染器注入到 Framework 委托层。
/// </summary>
public static class AvaloniaFrontendModule
{
    /// <summary>主编辑器窗口（由 App.OnFrameworkInitializationCompleted 创建）。</summary>
    internal static MainEditorWindow? MainWindow { get; set; }

    /// <summary>MainEditorWindow 就绪事件。</summary>
    private static readonly ManualResetEventSlim _windowReady = new(false);

    /// <summary>View 工厂实例。</summary>
    private static AvaloniaViewFactory? _viewFactory;

    /// <summary>Dock 服务实例。</summary>
    private static AvaloniaDockService? _dockService;

    /// <summary>窗口服务实例。</summary>
    private static AvaloniaWindowService? _windowService;

    /// <summary>通知服务实例。</summary>
    private static AvaloniaNotificationService? _notificationService;

    /// <summary>
    /// 安装主窗口。
    /// 必须在 Framework.Install() 之后、Core.Install() 之前调用。
    ///
    /// 与 ImGuiFrontendModule.InstallShell() 对应。
    /// </summary>
    public static void InstallShell(Neverness.Runtime.Application.Public.Window nativeWindow)
    {
        Console.WriteLine("[AvaloniaFrontendModule] 等待 Avalonia 主窗口就绪...");

        // 等待 MainEditorWindow 由 App.OnFrameworkInitializationCompleted 创建
        if (!_windowReady.Wait(TimeSpan.FromSeconds(10)))
        {
            Console.Error.WriteLine("[AvaloniaFrontendModule] 警告: 等待主窗口超时");
        }
        else
        {
            Console.WriteLine("[AvaloniaFrontendModule] 主窗口已就绪");
        }
    }

    /// <summary>
    /// 通知主窗口已创建（由 App.OnFrameworkInitializationCompleted 调用）。
    /// </summary>
    internal static void NotifyWindowReady()
    {
        _windowReady.Set();
    }

    /// <summary>
    /// 安装 AvaloniaFrontend 模块。
    /// 注入渲染器到 Framework 委托层。
    ///
    /// 与 ImGuiFrontendModule.Install() 对应。
    /// </summary>
    public static void Install()
    {
        Console.WriteLine("[AvaloniaFrontendModule] 安装 AvaloniaFrontend...");

        // 发现 Avalonia Inspector（扫描当前程序集中的 AvaloniaInspectorBase 实现）
        Inspectors.AvaloniaComponentInspectorRegistry.DiscoverFromAssembly(
            typeof(AvaloniaFrontendModule).Assembly);

        // 创建 View 工厂
        _viewFactory = new AvaloniaViewFactory();

        // 创建 FrontendServices 实例
        _dockService = new AvaloniaDockService();
        _windowService = new AvaloniaWindowService();
        _notificationService = new AvaloniaNotificationService();

        // 注册 FrontendServices 到服务定位器
        var context = CoreModuleImp.Context;
        context.RegisterService<IDockService>(_dockService);
        context.RegisterService<IWindowService>(_windowService);
        context.RegisterService<INotificationService>(_notificationService);

        // 注册 Avalonia ViewFactory 到 CompositionRoot
        EditorCompositionRoot.RegisterViewFactory(_viewFactory);

        // 设置 Avalonia 上下文菜单渲染器（ContextMenuManager → Avalonia ContextMenu）
        ContextMenuManager.Renderer = new AvaloniaContextMenuRenderer();

        // 刷新菜单栏（在 CoreModuleImp.Install() 之后，菜单贡献者已注册）
        if (MainWindow != null)
        {
            Dispatcher.UIThread.Invoke(() =>
            {
                MainWindow.RefreshMenuBar();
                Console.WriteLine("[AvaloniaFrontendModule] 菜单栏已刷新");
            });
        }

        Console.WriteLine("[AvaloniaFrontendModule] AvaloniaFrontend 安装完成");
    }

    /// <summary>
    /// 将 View 设置到 Dock 面板中。
    /// 必须在 EditorCompositionRoot.Build() 之后调用。
    /// </summary>
    public static void SetDockPanelContent()
    {
        if (MainWindow == null || _viewFactory == null)
        {
            Console.Error.WriteLine("[AvaloniaFrontendModule] 无法设置 Dock 面板内容: MainWindow 或 ViewFactory 未就绪");
            return;
        }

        Console.WriteLine($"[AvaloniaFrontendModule] 开始设置 Dock 面板内容...");
        Console.WriteLine($"[AvaloniaFrontendModule] SceneBrowserView: {_viewFactory.SceneBrowserView?.GetType().Name ?? "null"}");
        Console.WriteLine($"[AvaloniaFrontendModule] ViewportView: {_viewFactory.ViewportView?.GetType().Name ?? "null"}");
        Console.WriteLine($"[AvaloniaFrontendModule] InspectorView: {_viewFactory.InspectorView?.GetType().Name ?? "null"}");
        Console.WriteLine($"[AvaloniaFrontendModule] ContentBrowserView: {_viewFactory.ContentBrowserView?.GetType().Name ?? "null"}");
        Console.WriteLine($"[AvaloniaFrontendModule] ConsoleView: {_viewFactory.ConsoleView?.GetType().Name ?? "null"}");

        Dispatcher.UIThread.Invoke(() =>
        {
            // 将 View 添加到 MainEditorWindow 管理的 Panel 容器中
            // Document.Context 已经是 Panel，View 作为 Panel 的子控件
            if (_viewFactory.SceneBrowserView != null)
            {
                MainWindow.SetPanelContent(Dock.EditorDockFactory.PanelIds.SceneBrowser, _viewFactory.SceneBrowserView);
                Console.WriteLine($"[AvaloniaFrontendModule] 已设置 SceneBrowser 面板内容");
            }

            if (_viewFactory.ViewportView != null)
            {
                MainWindow.SetPanelContent(Dock.EditorDockFactory.PanelIds.Viewport, _viewFactory.ViewportView);
                Console.WriteLine($"[AvaloniaFrontendModule] 已设置 Viewport 面板内容");
            }

            if (_viewFactory.InspectorView != null)
            {
                MainWindow.SetPanelContent(Dock.EditorDockFactory.PanelIds.Inspector, _viewFactory.InspectorView);
                Console.WriteLine($"[AvaloniaFrontendModule] 已设置 Inspector 面板内容");
            }

            if (_viewFactory.ContentBrowserView != null)
            {
                MainWindow.SetPanelContent(Dock.EditorDockFactory.PanelIds.ContentBrowser, _viewFactory.ContentBrowserView);
                Console.WriteLine($"[AvaloniaFrontendModule] 已设置 ContentBrowser 面板内容");
            }

            if (_viewFactory.ConsoleView != null)
            {
                MainWindow.SetPanelContent(Dock.EditorDockFactory.PanelIds.Console, _viewFactory.ConsoleView);
                Console.WriteLine($"[AvaloniaFrontendModule] 已设置 Console 面板内容");
            }

            Console.WriteLine("[AvaloniaFrontendModule] Dock 面板内容已设置完成");
        });
    }

    /// <summary>注册上下文菜单贡献者（在 EditorCompositionRoot.Build() 之后调用）。</summary>
    public static void RegisterContextMenuContributors()
    {
        // 注册 ContentBrowser 背景/项目菜单（Create Directory、Refresh、Rename 等）
        // AssetCreationMenuContributor 由 AssetsModuleImp 自动注册，此处不重复
        EditorMenuRegistry.RegisterContextMenuContributor(
            new Neverness.Editor.Assets.Private.Context.ContentBrowserContextMenuContributor());
    }

    /// <summary>获取 Dock 服务。</summary>
    public static IDockService GetDockService() =>
        _dockService ?? throw new InvalidOperationException("AvaloniaFrontendModule not installed");

    /// <summary>获取窗口服务。</summary>
    public static IWindowService GetWindowService() =>
        _windowService ?? throw new InvalidOperationException("AvaloniaFrontendModule not installed");

    /// <summary>获取通知服务。</summary>
    public static INotificationService GetNotificationService() =>
        _notificationService ?? throw new InvalidOperationException("AvaloniaFrontendModule not installed");

    // ── 渲染回调注册 ──

    private static readonly List<Action> _renderCallbacks = new();

    /// <summary>注册主线程渲染回调。</summary>
    public static void RegisterRenderCallback(Action callback)
    {
        if (!_renderCallbacks.Contains(callback))
            _renderCallbacks.Add(callback);
    }

    /// <summary>注销主线程渲染回调。</summary>
    public static void UnregisterRenderCallback(Action callback)
    {
        _renderCallbacks.Remove(callback);
    }

    /// <summary>
    /// 主线程渲染 Tick——由 EditorApplicationRunner 每帧调用。
    /// 执行所有已注册的渲染回调（Diligent immediate context 非线程安全，必须主线程调用）。
    /// </summary>
    public static void TickRendering()
    {
        foreach (var callback in _renderCallbacks)
        {
            try
            {
                callback();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[AvaloniaFrontendModule] 渲染回调异常: {ex.Message}");
            }
        }
    }
}
