using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private;

namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器组装层——负责 ViewModel、Controller、View 的创建和连接。
///
/// 职责：
/// 1. 所有模块安装完成后执行
/// 2. 创建 ViewModel
/// 3. 创建 Controller（注入 Service 接口）
/// 4. 创建 View（由 Frontend 工厂提供）
/// 5. 注册到 PanelManager
///
/// 原则：
/// - ImGuiFrontend 不创建 ViewModel/Controller
/// - Core/Scene/Assets Module 不注册 UI
/// - Frontend 只负责 View
/// - CompositionRoot 负责对象组装
///
/// 未来 AvaloniaFrontend 只需替换 View 层。
/// </summary>
public static class EditorCompositionRoot
{
    // ── ViewModel 实例（供其他模块访问） ──
    public static SceneBrowserViewModel? SceneBrowserVM { get; private set; }
    public static InspectorViewModel? InspectorVM { get; private set; }
    public static EditorViewportViewModel? ViewportVM { get; private set; }
    public static ContentBrowserViewModel? ContentBrowserVM { get; private set; }
    public static ConsolePanelViewModel? ConsoleVM { get; private set; }

    // ── Controller 实例 ──
    public static SceneBrowserController? SceneBrowserController { get; private set; }
    public static InspectorController? InspectorController { get; private set; }
    public static EditorViewportController? ViewportController { get; private set; }
    public static ContentBrowserController? ContentBrowserController { get; private set; }

    // ── View 工厂（由 Frontend 模块注入） ──
    private static IViewFactory? _viewFactory;

    /// <summary>注册 View 工厂（由 ImGuiFrontend 或 AvaloniaFrontend 调用）。</summary>
    public static void RegisterViewFactory(IViewFactory factory)
    {
        _viewFactory = factory ?? throw new ArgumentNullException(nameof(factory));
    }

    /// <summary>
    /// 组装编辑器对象图。
    /// 在所有模块安装完成后调用。
    /// </summary>
    public static void Build()
    {
        Console.WriteLine("[EditorCompositionRoot] 开始组装编辑器对象图...");

        var context = CoreModuleImp.Context;
        var panelManager = PanelManager.Instance;

        // 1. 创建 ViewModel
        Console.WriteLine("[EditorCompositionRoot] 1. 创建 ViewModel...");
        CreateViewModels();

        // 2. 创建 Controller（注入 Service 接口）
        Console.WriteLine("[EditorCompositionRoot] 2. 创建 Controller...");
        CreateControllers(context);

        // 3. 初始化 Controller
        Console.WriteLine("[EditorCompositionRoot] 3. 初始化 Controller...");
        InitializeControllers();

        // 4. 创建 View 并注册到 PanelManager
        Console.WriteLine("[EditorCompositionRoot] 4. 创建 View 并注册到 PanelManager...");
        RegisterViews(panelManager);

        // 5. 注册 ViewModel 到服务定位器（供其他模块消费）
        Console.WriteLine("[EditorCompositionRoot] 5. 注册 Service...");
        RegisterServices(context);

        Console.WriteLine("[EditorCompositionRoot] 编辑器对象图组装完成");
    }

    /// <summary>关闭时清理资源。</summary>
    public static void Shutdown()
    {
        SceneBrowserController?.Shutdown();
        InspectorController?.Shutdown();
        ViewportController?.Shutdown();
        ContentBrowserController?.Shutdown();

        SceneBrowserVM = null;
        InspectorVM = null;
        ViewportVM = null;
        ContentBrowserVM = null;
        ConsoleVM = null;

        SceneBrowserController = null;
        InspectorController = null;
        ViewportController = null;
        ContentBrowserController = null;
    }

    // ── 内部方法 ──

    private static void CreateViewModels()
    {
        SceneBrowserVM = new SceneBrowserViewModel();
        InspectorVM = new InspectorViewModel();
        ViewportVM = new EditorViewportViewModel();
        ContentBrowserVM = new ContentBrowserViewModel();
        ConsoleVM = new ConsolePanelViewModel();
    }

    private static void CreateControllers(IEditorContext context)
    {
        // 通过服务定位器获取 Service 接口
        var sceneQueryService = context.GetService<ISceneQueryService>();
        var inspectorService = context.GetService<IInspectorService>();
        var viewportService = context.GetService<IViewportService>();
        var contentBrowserService = context.GetService<IContentBrowserService>();

        // 创建 Controller（注入 Service）
        SceneBrowserController = new SceneBrowserController(SceneBrowserVM!, sceneQueryService);
        InspectorController = new InspectorController(InspectorVM!, inspectorService);
        ViewportController = new EditorViewportController(ViewportVM!, viewportService);
        ContentBrowserController = new ContentBrowserController(ContentBrowserVM!, contentBrowserService);
    }

    private static void InitializeControllers()
    {
        SceneBrowserController?.Initialize();
        InspectorController?.Initialize();
        ViewportController?.Initialize();
        ContentBrowserController?.Initialize();

        // 建立 Controller 之间的连接：SceneBrowser 选中 → Inspector 刷新
        ConnectSceneBrowserToInspector();
    }

    /// <summary>连接 SceneBrowser 选中事件到 Inspector。</summary>
    private static void ConnectSceneBrowserToInspector()
    {
        if (SceneBrowserVM == null || InspectorController == null || InspectorVM == null) return;

        // 订阅 SceneBrowserViewModel 的选中变更事件
        SceneBrowserVM.PropertyChanged += (propertyName) =>
        {
            if (propertyName == nameof(SceneBrowserViewModel.SelectedEntityHandle))
            {
                var selectedHandle = SceneBrowserVM.SelectedEntityHandle;
                if (selectedHandle != 0)
                {
                    // 直接用 ISceneQueryService 获取实体信息（与 SceneBrowser 同源，避免 InspectorService 缓存不同步）
                    var sceneQueryService = CoreModuleImp.Context.GetService<ISceneQueryService>();
                    var sceneHandle = sceneQueryService.ActiveSceneHandle;
                    var entityName = sceneQueryService.GetEntityName(selectedHandle);
                    var entityData = sceneQueryService.GetEntity(selectedHandle);

                    // 直接更新 InspectorViewModel
                    InspectorVM.SetSelectedEntity(selectedHandle, entityName);
                    InspectorVM.SceneHandle = sceneHandle;
                    InspectorVM.IsActive = entityData?.IsActive ?? true;

                    // 刷新组件列表
                    InspectorController.RefreshComponents();
                }
                else
                {
                    InspectorController.ClearSelection();
                }
            }
        };
    }

    private static void RegisterViews(IPanelManager panelManager)
    {
        if (_viewFactory == null)
        {
            Console.WriteLine("[EditorCompositionRoot] 警告: ViewFactory 未注册，跳过 View 创建");
            return;
        }

        Console.WriteLine("[EditorCompositionRoot] ViewFactory 已注册，开始创建 View...");

        // 创建 View 并绑定 ViewModel
        Console.WriteLine("[EditorCompositionRoot] 创建 ConsoleView...");
        var consoleView = _viewFactory.CreateConsoleView(ConsoleVM!);

        Console.WriteLine("[EditorCompositionRoot] 创建 SceneBrowserView...");
        var sceneBrowserView = _viewFactory.CreateSceneBrowserView(SceneBrowserVM!, SceneBrowserController!);

        Console.WriteLine("[EditorCompositionRoot] 创建 InspectorView...");
        var inspectorView = _viewFactory.CreateInspectorView(InspectorVM!, InspectorController!);

        Console.WriteLine("[EditorCompositionRoot] 创建 ViewportView...");
        var viewportView = _viewFactory.CreateViewportView(ViewportVM!, ViewportController!);

        Console.WriteLine("[EditorCompositionRoot] 创建 ContentBrowserView...");
        var contentBrowserView = _viewFactory.CreateContentBrowserView(ContentBrowserVM!, ContentBrowserController!);

        // 注册到 PanelManager
        Console.WriteLine("[EditorCompositionRoot] 注册面板到 PanelManager...");
        var result1 = panelManager.AddChildPanel("Console", consoleView);
        Console.WriteLine($"[EditorCompositionRoot] Console 面板注册结果: {result1}");

        var result2 = panelManager.AddChildPanel("SceneBrowser", sceneBrowserView);
        Console.WriteLine($"[EditorCompositionRoot] SceneBrowser 面板注册结果: {result2}");

        var result3 = panelManager.AddChildPanel("DetailInspector", inspectorView);
        Console.WriteLine($"[EditorCompositionRoot] DetailInspector 面板注册结果: {result3}");

        var result4 = panelManager.AddChildPanel("EditorViewport", viewportView);
        Console.WriteLine($"[EditorCompositionRoot] EditorViewport 面板注册结果: {result4}");

        var result5 = panelManager.AddChildPanel("ContentBrowser", contentBrowserView);
        Console.WriteLine($"[EditorCompositionRoot] ContentBrowser 面板注册结果: {result5}");
    }

    private static void RegisterServices(IEditorContext context)
    {
        // 注册 ViewModel 到服务定位器（供其他模块消费）
        context.RegisterService(SceneBrowserVM!);
        context.RegisterService(InspectorVM!);
        context.RegisterService(ViewportVM!);
        context.RegisterService(ContentBrowserVM!);
        context.RegisterService(ConsoleVM!);

        // 注册 Controller 到服务定位器
        context.RegisterService(SceneBrowserController!);
        context.RegisterService(InspectorController!);
        context.RegisterService(ViewportController!);
        context.RegisterService(ContentBrowserController!);
    }
}

/// <summary>
/// View 工厂接口——由 Frontend 模块实现。
/// ImGuiFrontend 和 AvaloniaFrontend 各自提供实现。
/// </summary>
public interface IViewFactory
{
    IEditorPanel CreateConsoleView(ConsolePanelViewModel viewModel);
    IEditorPanel CreateSceneBrowserView(SceneBrowserViewModel viewModel, SceneBrowserController controller);
    IEditorPanel CreateInspectorView(InspectorViewModel viewModel, InspectorController controller);
    IEditorPanel CreateViewportView(EditorViewportViewModel viewModel, EditorViewportController controller);
    IEditorPanel CreateContentBrowserView(ContentBrowserViewModel viewModel, ContentBrowserController controller);
}
