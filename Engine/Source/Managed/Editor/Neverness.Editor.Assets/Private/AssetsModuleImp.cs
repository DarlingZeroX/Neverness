using Neverness.Editor.Assets.AssetActions;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Assets.Private.Core;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Scene;
using Neverness.Runtime.VFS;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets.Private;

/// <summary>
/// 资产工厂模块内部安装实现。
/// 负责：ContentBrowser 引擎、上下文菜单贡献者、资产工厂系统、热重载。
///
/// 注意：ContentBrowser 面板注册已移至 EditorCompositionRoot，此处不再注册 UI 面板。
/// </summary>
internal static class AssetsModuleImp
{
    private static HotReloadCoordinator? s_hotReloadCoordinator;
    private static DropImportService? s_dropImportService;

    public static void Install(SceneManager sceneManager)
    {
        // 1. 初始化 ContentBrowser 引擎
        string? path = VFSService.GetAbsolutePath(ProjectPaths.Assets.FullPath);
        if (path != null)
        {
            ContentBrowser.Create(path);
        }

        // 1.1 注册 ContentBrowserService 到编辑器上下文（供 Scene / Inspector 等模块消费）
        if (ContentBrowser.Instance != null)
        {
            EditorCoreModule.Context.RegisterService<IContentBrowserService>(
                new ContentBrowserService());
        }

        // 1.2 初始化 EditorAssetDatabase（路径索引、类型查询基础）
        var libraryPath = VFSService.GetAbsolutePath(ProjectPaths.Library.FullPath);
        if (path != null && libraryPath != null)
        {
            EditorAssetDatabase.Initialize(new NPath(path), new NPath(libraryPath));
        }

        // 2. 初始化 Asset Open Pipeline（在面板创建前注册服务）
        EditorCoreModule.Context.RegisterService(sceneManager);
        var assetEditorManager = new AssetEditorManager();
        EditorCoreModule.Context.RegisterService(assetEditorManager);
        var openerRegistry = new AssetOpenerRegistry();
        openerRegistry.Discover(EditorCoreModule.Context);
        EditorCoreModule.Context.RegisterService(openerRegistry);
        var openService = new AssetOpenService(openerRegistry);
        EditorCoreModule.Context.RegisterService(openService);

        // 2.1 注册 IAssetOpenService 到编辑器上下文（供 Controller 消费）
        EditorCoreModule.Context.RegisterService<IAssetOpenService>(
            new AssetOpenServiceImpl(openService));

        // 3. 注册 ContentBrowser 上下文菜单贡献者
        // 已移至 ImGuiFrontend 模块的 ContentBrowserImGuiContextMenuContributor

        // 4. 触发工厂自动发现（扫描所有 Neverness.Editor.* 程序集）
        var factories = AssetFactoryRegistry.Instance.Factories;

        // 5. 注册资产类型操作元数据
        var actions = AssetTypeActionsRegistry.Instance;
        foreach (var factory in factories)
        {
            actions.Register(new AssetTypeActions
            {
                DisplayName = factory.DisplayName,
                FileExtension = factory.FileExtension,
                Icon = factory.Icon,
                Category = factory.Category,
                CanOpen = true,
                CanImport = false,
            });
        }

        // 6. 注册资产创建上下文菜单贡献者
        EditorMenuRegistry.RegisterContextMenuContributor(new AssetCreationMenuContributor());

        // 7. 初始化 ImportPipeline 并启动热重载
        if (path != null)
        {
            if (libraryPath != null)
            {
                ImportPipeline.Initialize(new NPath(libraryPath));

                // 初始化 C# AssetManager（projectRoot = Library 的父目录）
                var projectRoot = System.IO.Path.GetDirectoryName(libraryPath);
                if (projectRoot != null)
                    AssetManager.Instance.Initialize(projectRoot);
            }

            s_hotReloadCoordinator = new HotReloadCoordinator(new NPath(path));
            s_hotReloadCoordinator.Start();

            // 8. 启动 DropImportService（拖放外部文件导入）
            s_dropImportService = new DropImportService(EditorCoreModule.Context.Events, path);
            s_dropImportService.Start();

            // 8.1 注册 IDropImportService 到编辑器上下文（供 Controller 消费）
            EditorCoreModule.Context.RegisterService<IDropImportService>(
                new DropImportServiceImp());
        }

    }

    /// <summary>每帧 Tick——批量保存脏缓存。</summary>
    public static void Tick()
    {
        EditorAssetDatabase.SaveIfDirty();
    }

    /// <summary>关闭模块（停止热重载、强制保存）。</summary>
    public static void Shutdown()
    {
        s_dropImportService?.Stop();
        s_dropImportService = null;
        s_hotReloadCoordinator?.Dispose();
        s_hotReloadCoordinator = null;
        EditorAssetDatabase.SaveIfDirty();
        if (EditorCoreModule.Context.TryGetService<AssetEditorManager>(out var mgr))
            mgr.Clear();
    }
}
