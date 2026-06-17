using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.PlayMode;
using Neverness.Editor.Scene.Private.Panel;
using Neverness.Editor.Scene.Private.Inspector;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Engine;
using Neverness.Editor.Scene.Private.Service;
using Neverness.Editor.Core;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Public.Inspector;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// 场景编辑模块内部安装实现。
/// 负责：Service 注册、PlayMode 控制器、场景事件桥接。
///
/// 注意：面板注册已移至 EditorCompositionRoot，此处不再注册 UI 面板。
/// </summary>
internal static class SceneModuleImp
{
    private static SceneEditorBridge? s_bridge;

    /// <summary>播放模式控制器——驱动 PlayMode 状态机。</summary>
    public static PlayModeController PlayModeController { get; private set; } = null!;

    /// <summary>场景查询服务实例。</summary>
    public static SceneQueryServiceImpl? SceneQueryService { get; private set; }

    /// <summary>Inspector 服务实例。</summary>
    public static InspectorServiceImpl? InspectorService { get; private set; }

    /// <summary>视口服务实例。</summary>
    public static ViewportServiceImpl? ViewportService { get; private set; }

    /// <summary>视口表面注册表实例。</summary>
    public static ViewportSurfaceRegistryImpl? ViewportSurfaceRegistry { get; private set; }

    /// <summary>获取层级缓存（Debug / 诊断用，通过 Service 访问）。</summary>
    public static Cache.SceneHierarchyCache? HierarchyCache => SceneQueryService?.Cache;

    /// <summary>安装场景编辑模块（场景句柄后续设置）。</summary>
    public static void Install(SceneManager sceneManager)
    {
        // 创建播放模式控制器
        var context = Neverness.Editor.Core.Public.EditorCoreModule.Context;
        PlayModeController = new PlayModeController(sceneManager, context.State, context.Events);

        // 注入 TickOverride，让 PlayModeController 驱动 ECS tick（按标签过滤）
        SceneSubsystem.TickOverride = deltaTime =>
        {
            PlayModeController.TickActiveScene(deltaTime);
            return true;
        };

        // 创建并注册 Service 实现（Controller 通过服务定位器消费）
        SceneQueryService = new SceneQueryServiceImpl();
        InspectorService = new InspectorServiceImpl();
        ViewportService = new ViewportServiceImpl();
        ViewportSurfaceRegistry = new ViewportSurfaceRegistryImpl();

        // 注册通用组件检查器（使 ComponentInspectorRegistry 能枚举实体组件）
        // Avalonia 前端不加载 ImGuiFrontend，需要在此注册
        RegisterGenericInspectors();

        context.RegisterService<ISceneQueryService>(SceneQueryService);
        context.RegisterService<IInspectorService>(InspectorService);
        context.RegisterService<IViewportService>(ViewportService);
        context.RegisterService<IViewportSurfaceRegistry>(ViewportSurfaceRegistry);

        // 注册上下文菜单贡献者
        EditorMenuRegistry.RegisterContextMenuContributor(new SceneBrowserContextMenuContributor());

        // 创建场景编辑器桥接（事件驱动）
        s_bridge = new SceneEditorBridge(sceneManager);

        // 注册 PlayMode 命令（Shell 通过命令系统调用，不直接引用 Scene 模块）
        RegisterPlayModeCommands();

        // 注册 Scene 相关菜单项（从 BuiltinMenuContributor 移入）
        RegisterSceneMenuItems(sceneManager);
    }

    /// <summary>注册 Scene 相关菜单项和命令。</summary>
    private static void RegisterSceneMenuItems(SceneManager sceneManager)
    {
        // File/New Scene 命令
        var newSceneCommand = new EditorCommand
        {
            Id = "file.new",
            DisplayName = "New Scene",
            Execute = _ =>
            {
                Console.WriteLine("[Scene] 创建新场景");
                // TODO: 实现新场景创建逻辑
            },
        };
        EditorMenuRegistry.RegisterCommand(newSceneCommand);
        EditorMenuRegistry.Register(new EditorMenuItem(
            "File/New Scene",
            Command: newSceneCommand,
            Icon: "📄",
            SortOrder: 100));

        // File/Open Scene 命令
        var openSceneCommand = new EditorCommand
        {
            Id = "file.open",
            DisplayName = "Open Scene",
            Execute = _ =>
            {
                Console.WriteLine("[Scene] 打开场景");
                // TODO: 实现场景打开逻辑（文件对话框）
            },
        };
        EditorMenuRegistry.RegisterCommand(openSceneCommand);
        EditorMenuRegistry.Register(new EditorMenuItem(
            "File/Open Scene",
            Command: openSceneCommand,
            Icon: "📂",
            SortOrder: 200));

        // File/Save Scene 命令
        var saveCommand = new EditorCommand
        {
            Id = "file.save",
            DisplayName = "Save Scene",
            Execute = _ =>
            {
                var result = sceneManager.SaveActiveScene();
                if (result.IsSuccess())
                {
                    Console.WriteLine("[Scene] 场景已保存");
                }
                else
                {
                    Console.WriteLine($"[Scene] 保存失败: {result}");
                    // TODO: 弹出 Save As 对话框
                }
            },
            CanExecute = () => sceneManager.HasActiveScene,
        };
        EditorMenuRegistry.RegisterCommand(saveCommand);
        EditorMenuRegistry.Register(new EditorMenuItem(
            "File/Save Scene",
            Command: saveCommand,
            Icon: "💾",
            Shortcut: "Ctrl+S",
            SortOrder: 300));

        // File/Save Scene As... 命令
        var saveAsCommand = new EditorCommand
        {
            Id = "file.saveAs",
            DisplayName = "Save Scene As...",
            Execute = _ =>
            {
                Console.WriteLine("[Scene] 场景另存为");
                // TODO: 实现场景另存为逻辑（文件对话框）
            },
            CanExecute = () => sceneManager.HasActiveScene,
        };
        EditorMenuRegistry.RegisterCommand(saveAsCommand);
        EditorMenuRegistry.Register(new EditorMenuItem(
            "File/Save Scene As...",
            Command: saveAsCommand,
            Icon: "💾",
            SortOrder: 400));
    }

    /// <summary>设置场景（场景切换时调用）。</summary>
    public static void SetScene(SceneWorld? world)
    {
        // 同步场景到 Service 层
        SceneQueryService?.SetActiveScene(world);
        ViewportService?.SetScene(world);

        // 同步到 ViewModel 和 Controller（通过 EditorCompositionRoot）
        var sceneBrowserVM = EditorCompositionRoot.SceneBrowserVM;
        var sceneBrowserCtrl = EditorCompositionRoot.SceneBrowserController;
        var viewportVM = EditorCompositionRoot.ViewportVM;
        var viewportCtrl = EditorCompositionRoot.ViewportController;

        if (sceneBrowserVM != null)
        {
            sceneBrowserVM.HasScene = world != null;
        }

        if (viewportVM != null)
        {
            viewportVM.HasScene = world != null;
        }

        // 刷新场景浏览器并重新订阅事件
        if (world != null)
        {
            sceneBrowserCtrl?.OnSceneChanged();
            sceneBrowserCtrl?.RefreshTree();
        }
    }

    /// <summary>注册 PlayMode 相关命令（Shell 通过命令系统调用）。</summary>
    private static void RegisterPlayModeCommands()
    {
        // scene.play — 进入播放模式 / 恢复播放
        EditorMenuRegistry.RegisterCommand(new EditorCommand
        {
            Id = "scene.play",
            DisplayName = "Play",
            Execute = _ =>
            {
                if (PlayModeController.IsPaused)
                    PlayModeController.Resume();
                else
                    PlayModeController.EnterPlay();
            },
            CanExecute = () => PlayModeController.CurrentMode != Editor.Core.Public.PlayMode.Playing,
            IsChecked = () => PlayModeController.IsPlaying || PlayModeController.IsPaused,
        });

        // scene.stop — 退出播放模式
        EditorMenuRegistry.RegisterCommand(new EditorCommand
        {
            Id = "scene.stop",
            DisplayName = "Stop",
            Execute = _ => PlayModeController.ExitPlay(),
            CanExecute = () => PlayModeController.CurrentMode != Editor.Core.Public.PlayMode.Editing,
        });

        // scene.pause — 暂停
        EditorMenuRegistry.RegisterCommand(new EditorCommand
        {
            Id = "scene.pause",
            DisplayName = "Pause",
            Execute = _ => PlayModeController.Pause(),
            CanExecute = () => PlayModeController.CurrentMode == Editor.Core.Public.PlayMode.Playing,
        });
    }

    /// <summary>注册通用组件检查器到 ComponentInspectorRegistry。
    /// TypeId 须与 Native NN_REGISTER_COMPONENT 的 FNV-1a hash 一致。</summary>
    private static void RegisterGenericInspectors()
    {
        ComponentInspectorRegistry.Register(new GenericComponentInspector<TransformComponent>(
            0xC1FFF4F356DFB2FB, "Transform", order: 0));
        ComponentInspectorRegistry.Register(new GenericComponentInspector<CameraComponent>(
            0x54D1B2A64667E32E, "Camera", order: 10));
        ComponentInspectorRegistry.Register(new GenericComponentInspector<SpriteRendererComponent>(
            0x51387BA3968C343B, "SpriteRenderer", order: 20));
        ComponentInspectorRegistry.Register(new GenericComponentInspector<RmlUIDocumentComponent>(
            0x1593AE057DEB826B, "RmlUI Document", order: 70));

        Console.WriteLine($"[SceneModule] 已注册 {ComponentInspectorRegistry.Inspectors.Count} 个通用组件检查器");
    }
}
