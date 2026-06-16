using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.PlayMode;
using Neverness.Editor.Scene.Private.Panel;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Engine;
using Neverness.Editor.Scene.Private.Service;
using Neverness.Editor.Core;
using Neverness.Editor.Core.Public;

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

        // 注册 Save Scene 命令
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

        // 覆盖菜单项，绑定命令（BuiltinMenuContributor 注册的无 Command）
        EditorMenuRegistry.Register(new EditorMenuItem(
            "File/Save Scene",
            Command: saveCommand,
            Icon: FontAwesome5Pro.Save,
            Shortcut: "Ctrl+S",
            SortOrder: 300));
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
}
