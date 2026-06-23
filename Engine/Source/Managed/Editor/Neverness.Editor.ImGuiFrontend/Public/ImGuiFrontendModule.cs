using Neverness.Editor.Framework.Public;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.ImGuiFrontend.Menu;
using Neverness.Editor.ImGuiFrontend.DragDrop;
using Neverness.Editor.ImGuiFrontend.AssetOpening;
using Neverness.Editor.ImGuiFrontend.ContextMenus;
using Neverness.Editor.ImGuiFrontend.Shell;
using Neverness.Editor.Media;
using Neverness.Editor.ImGuiEx;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.ImGuiFrontend.Public;

/// <summary>
/// ImGuiFrontend 模块入口——注册 ImGui 渲染器实现到 Framework 委托层。
///
/// 在 NevernessEditor 启动时调用，将 ImGui 特定的渲染器注入到
/// ContextMenuManager.Renderer 和 AssetDragDrop.Service，
/// 使旧代码通过委托层间接调用 ImGui 渲染，无需直接引用 ImGui。
/// </summary>
public static class ImGuiFrontendModule
{
    private static ImGuiContextMenuRenderer? _contextMenuRenderer;
    private static ImGuiAssetDragDropService? _dragDropService;
    private static ImGuiToolbarRendererWrapper? _toolbarRenderer;

    /// <summary>
    /// 安装 ImGuiFrontend 模块。
    /// 注入渲染器到 Framework 的委托层。
    /// </summary>
    public static void Install()
    {
        // 创建渲染器实例
        _contextMenuRenderer = new ImGuiContextMenuRenderer();
        _dragDropService = new ImGuiAssetDragDropService();
        _toolbarRenderer = new ImGuiToolbarRendererWrapper();

        // 注入到 Framework 委托层
        ContextMenuManager.Renderer = _contextMenuRenderer;
        AssetDragDrop.Service = _dragDropService;

        // 注册 ImGui ViewFactory 到 CompositionRoot
        EditorCompositionRoot.RegisterViewFactory(new ImGuiViewFactory());

        // 扫描当前程序集中的组件检查器
        ComponentInspectorRegistry.DiscoverFromAssembly(typeof(ImGuiFrontendModule).Assembly);

        // 注册查看器服务（供 Media 模块的 AssetOpener 使用）
        RegisterViewerServices();
    }

    /// <summary>
    /// 安装主窗口和菜单栏（原 Shell 模块职责）。
    /// 必须在 Framework.Install() 之后、Core.Install() 之前调用。
    /// </summary>
    public static void InstallShell(SdlWindow window)
    {
        var panelManager = PanelManager.Instance;

        // 创建主窗口和菜单栏
        var mainWindow = new ImGuiMainWindow();
        var menuBar = new ImGuiMenuBar(window);

        // 注册到 PanelManager
        panelManager.AddPanelWithID("EditorMainWindow", mainWindow);
        panelManager.AddPanelWithID("EditorMenuBar", menuBar);

        // 注册 AddChildPanel 回调，使 PanelManager 能委托子面板注册到主窗口
        panelManager.RegisterMainWindowCallback((id, panel) =>
        {
            mainWindow.AddPanelWithID(id, panel);
            return true;
        });
    }

    /// <summary>注册查看器服务。</summary>
    private static void RegisterViewerServices()
    {
        var context = EditorCoreModule.Context;

        // 获取依赖的服务
        if (context.TryGetService<IImWindowManager>(out var windowManager) &&
            context.TryGetService<AssetEditorManager>(out var editorManager))
        {
            context.RegisterService<IVideoViewerService>(
                new VideoViewerServiceImpl(windowManager, editorManager));

            context.RegisterService<IAudioViewerService>(
                new AudioViewerServiceImpl(windowManager, editorManager));
        }
    }

    /// <summary>注册上下文菜单贡献者（在 EditorCompositionRoot.Build() 之后调用）。</summary>
    public static void RegisterContextMenuContributors()
    {
        var contentBrowserController = EditorCompositionRoot.ContentBrowserController;
        if (contentBrowserController != null)
        {
            // 注册 ContentBrowser 的上下文菜单贡献者（使用 Controller 执行操作）
            EditorMenuRegistry.RegisterContextMenuContributor(
                new ContentBrowserImGuiContextMenuContributor(contentBrowserController));

            // 注册资产创建菜单贡献者
            EditorMenuRegistry.RegisterContextMenuContributor(
                new AssetCreationImGuiMenuContributor(contentBrowserController));
        }
    }

    /// <summary>
    /// 获取工具栏渲染器（供其他模块使用）。
    /// </summary>
    public static ImGuiToolbarRendererWrapper GetToolbarRenderer() =>
        _toolbarRenderer ?? throw new InvalidOperationException("ImGuiFrontendModule not installed");
}
