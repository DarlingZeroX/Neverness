// ============================================================================
// ScriptEditorModule.cs - 脚本编辑器模块入口
// ============================================================================
// EditorApplicationRunner 调用 Install() 初始化所有脚本相关功能。
//
// 职责：
// - 生成工程文件（Game.sln + Assembly-CSharp.csproj）—— IDE 用
// - 初始化 Roslyn 编译服务
// - 后台编译脚本
// - Play Mode 生命周期：加载已编译 Assembly → 注册系统
// ============================================================================

using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.VFS;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Editor.Script.Private;
using Neverness.Editor.Scene.Private.PlayMode;
using Neverness.Editor.Scene.Public;
using Neverness.Gameplay;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Application.Public;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scripting;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本编辑器模块入口。
/// </summary>
public static class ScriptEditorModule
{
    private static ScriptEditorServiceImpl? _service;
    private static ScriptCompileService? _compileService;
    private static ScriptProjectGenerator? _projectGenerator;
    private static ScriptAssemblyDefinition? _assemblyDefinition;
    private static GameplayContext? _gameplayContext;
    private static ScriptBehaviourBridge? _bridge;

    /// <summary>
    /// 输入提供者（从 ApplicationModule 获取）。
    /// EnterPlayMode 时绑定到 Viewport 窗口并传给 GameplayContext。
    /// </summary>
    private static SdlInputProvider? InputProvider => ApplicationModule.InputProvider;

    /// <summary>当前绑定的 Viewport 窗口（用于 ExitPlayMode 时解绑）。</summary>
    private static SdlWindow? _inputWindow;

    /// <summary>安装脚本编辑器模块。</summary>
    public static void Install(IEditorContext context)
    {
        // ── 路径解析 ──
        var projectRoot = VFSService.GetAbsolutePath(ProjectPaths.Project.FullPath);
        var assetsDir = VFSService.GetAbsolutePath(ProjectPaths.Assets.FullPath);
        var libraryDir = VFSService.GetAbsolutePath(ProjectPaths.Library.FullPath);

        if (string.IsNullOrEmpty(projectRoot) || string.IsNullOrEmpty(assetsDir) || string.IsNullOrEmpty(libraryDir))
        {
            Console.WriteLine("[ScriptEditorModule] VFSService paths not resolvable, script system disabled");
            return;
        }

        // ── 引擎程序集解析器 ──
        var engineBinariesDir = GetEngineBinariesDir();
        var engineResolver = new DefaultEngineAssemblyResolver(engineBinariesDir, projectRoot);

        // ── 程序集定义 ──
        _assemblyDefinition = new ScriptAssemblyDefinition
        {
            Name = "Assembly-CSharp",
            SourceGlobs = new[] { @"Assets\**\*.cs" },
            References = new[]
            {
                "Friflo.Engine.ECS",
                "Neverness.Gameplay",
                "Neverness.Runtime.Scene",
                "Neverness.Runtime.Engine",
                "Neverness.Runtime.Assets",
                "Neverness.Runtime.Engine",
                "Neverness.Runtime.Application",
            },
            OutputPath = Path.Combine(libraryDir, "Scripts"),
        };

        // ── 工程文件生成（IDE 用，不参与编译）──
        _projectGenerator = new ScriptProjectGenerator(engineResolver);
        _projectGenerator.GenerateProjectFiles(projectRoot, new[] { _assemblyDefinition });

        // ── Roslyn 编译器 ──
        var compiler = new RoslynScriptCompiler();
        var refProvider = new TpaReferenceProvider();

        // ── 编译服务 ──
        _compileService = new ScriptCompileService(compiler, refProvider);

        // ── 编译完成后：刷新 ScriptAssetIndex + 更新服务状态 ──
        _compileService.CompileFinished += (_, roslynResult) =>
        {
            ScriptAssetIndex.Instance.RefreshAllTypeIds();

            // 转换为 Editor.Script.Public.ScriptCompileResult
            var editorResult = new ScriptCompileResult
            {
                Status = roslynResult.Success ? ScriptCompileStatus.Success : ScriptCompileStatus.Error,
                Diagnostics = roslynResult.Diagnostics
                    .Where(d => d.Severity == Microsoft.CodeAnalysis.DiagnosticSeverity.Error ||
                                d.Severity == Microsoft.CodeAnalysis.DiagnosticSeverity.Warning)
                    .Select(d => new ScriptDiagnostic
                    {
                        FilePath = d.Location.SourceTree?.FilePath ?? "",
                        Line = d.Location.GetLineSpan().StartLinePosition.Line,
                        Column = d.Location.GetLineSpan().StartLinePosition.Character,
                        Message = d.GetMessage(),
                        IsError = d.Severity == Microsoft.CodeAnalysis.DiagnosticSeverity.Error
                    })
                    .ToArray(),
                Duration = roslynResult.Duration
            };

            _service?.UpdateCompileStatus(editorResult.Status, editorResult);
            Console.WriteLine("[ScriptEditorModule] ScriptAssetIndex refreshed after compilation");
        };

        // ── 后台预编译 ──
        _compileService.CompileAsync(_assemblyDefinition);

        // ── 服务 ──
        _service = new ScriptEditorServiceImpl();
        _service.SetWatchDirectory(assetsDir);
        _service.SetAssemblyDefinition(_assemblyDefinition);
        _service.StartWatching(assetsDir);

        context.RegisterService<IScriptEditorService>(_service);

        // ── 脚本文件变化时标记 Dirty + 后台编译 ──
        _service.FileChanged += (_, e) =>
        {
            _compileService.MarkSourceChanged();

            // 文件增删时重新生成工程文件
            if (e.ChangeKind is ScriptFileChangeKind.Created or ScriptFileChangeKind.Deleted)
            {
                _projectGenerator.GenerateProjectFiles(projectRoot!, new[] { _assemblyDefinition! });
            }

            // 后台编译
            _compileService.CompileAsync(_assemblyDefinition!);
        };

        // ── Play Mode 生命周期 ──
        context.Events.Subscribe(EditorEventType.PlayModeChanged, OnPlayModeChanged);

        // ── 资产工厂 ──
        AssetFactoryRegistry.Instance.Register(new CSharpScriptAssetFactory());

        // ── 脚本资产索引 ──
        ScriptAssetIndex.Instance.RebuildAll();

        // ── ECS 脚本组件 Inspector ──
        // 已移至 ImGuiFrontend 模块，由 ImGuiFrontendModule 自动扫描注册

        // ── 菜单命令 ──
        RegisterMenuCommands(context);

        Console.WriteLine($"[ScriptEditorModule] Initialized. Project root: {projectRoot}");
    }

    /// <summary>获取脚本编辑器服务。</summary>
    public static IScriptEditorService Service =>
        _service ?? throw new InvalidOperationException("ScriptEditorModule is not installed.");

    /// <summary>关闭脚本编辑器模块。</summary>
    public static void Shutdown()
    {
        _service?.Dispose();
        _service = null;
        _compileService = null;
        _projectGenerator = null;
        _assemblyDefinition = null;
        _gameplayContext = null;
        _bridge = null;
    }

    // ========================================================================
    // Play Mode 生命周期
    // ========================================================================

    private static async void OnPlayModeChanged(EditorEvent evt)
    {
        if (evt.Payload is not PlayModeChangedEvent playModeEvent)
            return;

        if (playModeEvent.NewMode == PlayMode.Playing)
        {
            await EnterPlayMode();
        }
        else if (playModeEvent.OldMode == PlayMode.Playing &&
                 playModeEvent.NewMode == PlayMode.Editing)
        {
            ExitPlayMode();
        }
    }

    /// <summary>
    /// 进入播放模式：确保编译完成 → 加载 Assembly → 初始化 GameplayContext → 注册系统。
    /// </summary>
    private static async Task EnterPlayMode()
    {
        Console.WriteLine("[ScriptEditorModule] Entering play mode...");

        // 1. 确保有编译结果（Dirty 或无缓存时同步编译）
        if (_compileService != null && (_compileService.IsDirty || _compileService.LastResult == null))
        {
            Console.WriteLine("[ScriptEditorModule] Compiling scripts...");
            var compileResult = _compileService.CompileSync(_assemblyDefinition!);
            if (!compileResult.Success)
            {
                Console.WriteLine("[ScriptEditorModule] ❌ Script compilation failed, play mode aborted");
                foreach (var diag in compileResult.Diagnostics.Where(d => d.Severity == Microsoft.CodeAnalysis.DiagnosticSeverity.Error))
                {
                    Console.WriteLine($"  ERROR: {diag.GetMessage()}");
                }
                return;
            }
        }

        // 2. 初始化 GameplayContext
        _gameplayContext = new GameplayContext();
        if (InputProvider != null)
        {
            // 将 InputProvider 绑定到 Viewport 窗口的事件
            AttachInputProviderToViewport();
            _gameplayContext.InputProvider = InputProvider;
        }
        _gameplayContext.Initialize();
        Console.WriteLine("[ScriptEditorModule] GameplayContext initialized");

        // 3. 加载已编译 Assembly（从内存）
        if (_compileService != null)
        {
            if (!_compileService.LoadFromLastResult())
            {
                Console.WriteLine("[ScriptEditorModule] ❌ No compiled assembly available, play mode aborted");
                return;
            }
        }

        // 4. 获取活动 SceneWorld
        var world = Neverness.Editor.Scene.Public.SceneModule.GetActiveWorld();
        if (world == null || !world.IsValid)
        {
            Console.WriteLine("[ScriptEditorModule] No active SceneWorld, systems not registered");
            return;
        }

        // 5. 注册 ScriptBehaviourScheduler 到 SceneWorld
        var scheduler = _gameplayContext.BehaviourScheduler;
        world.Systems.Register(scheduler);
        Console.WriteLine("[ScriptEditorModule] ScriptBehaviourScheduler registered");

        // 6. 创建并注册 ScriptBehaviourBridge
        var factory = new DefaultScriptFactory();
        _bridge = new ScriptBehaviourBridge(
            factory,
            scheduler.Registry,
            _gameplayContext.ScriptRegistry,
            scheduler,
            () => Neverness.Editor.Scene.Public.SceneModule.GetActiveWorld());
        world.Systems.Register(_bridge);
        Console.WriteLine("[ScriptEditorModule] ScriptBehaviourBridge registered");

        Console.WriteLine("[ScriptEditorModule] ✅ Play mode ready");
    }

    /// <summary>退出播放模式：销毁 GameplayContext。</summary>
    private static void ExitPlayMode()
    {
        Console.WriteLine("[ScriptEditorModule] Exiting play mode...");

        // 解绑 InputProvider
        DetachInputProvider();

        _bridge = null;

        if (_gameplayContext != null)
        {
            _gameplayContext.Shutdown();
            _gameplayContext = null;
            Console.WriteLine("[ScriptEditorModule] GameplayContext shutdown");
        }

        Console.WriteLine("[ScriptEditorModule] ✅ Play mode exited");
    }

    /// <summary>
    /// 将 InputProvider 绑定到 Viewport 窗口的事件。
    /// 从 ViewportIdManager 获取第一个 Viewport 的 SdlWindow。
    /// </summary>
    private static void AttachInputProviderToViewport()
    {
        if (InputProvider == null) return;

        // 从 ViewportIdManager 获取游戏视口
        var viewport = ViewportIdManager.GetGameViewportId();
        if (!viewport.IsValid)
        {
            Console.WriteLine("[ScriptEditorModule] 无可用 Viewport，InputProvider 未绑定窗口");
            return;
        }

        var window = viewport.Window;
        if (window == null)
        {
            Console.Error.WriteLine($"[ScriptEditorModule] 无法解析 Viewport 窗口: {viewport.WindowHandle}");
            return;
        }

        InputProvider.Attach(window.Events);
        _inputWindow = window;
        Console.WriteLine($"[ScriptEditorModule] InputProvider 已绑定到 Viewport 窗口: {viewport.WindowHandle}");
    }

    /// <summary>解绑 InputProvider。</summary>
    private static void DetachInputProvider()
    {
        if (InputProvider != null && _inputWindow != null)
        {
            InputProvider.Detach(_inputWindow.Events);
            Console.WriteLine("[ScriptEditorModule] InputProvider 已解绑");
        }
        _inputWindow = null;
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    private static void RegisterMenuCommands(IEditorContext context)
    {
        var compileAllCmd = new EditorCommand
        {
            Id = "script.compile_all",
            DisplayName = "Compile All Scripts",
            Execute = _ =>
            {
                if (_compileService != null && _assemblyDefinition != null)
                    _compileService.CompileAsync(_assemblyDefinition);
            },
            Tooltip = "编译所有 Gameplay 脚本"
        };

        var hotReloadCmd = new EditorCommand
        {
            Id = "script.hot_reload",
            DisplayName = "Hot Reload",
            Execute = _ =>
            {
                if (_compileService != null && _assemblyDefinition != null)
                    _compileService.HotReload(_assemblyDefinition);
            },
            Tooltip = "请求热重载脚本"
        };

        context.Menus.RegisterCommand(compileAllCmd);
        context.Menus.RegisterCommand(hotReloadCmd);

        context.Menus.Register(new EditorMenuItem
        {
            Path = "Scripts/Compile All Scripts",
            Command = compileAllCmd,
            Shortcut = "F7",
            SortOrder = 100
        });

        context.Menus.Register(new EditorMenuItem
        {
            Path = "Scripts/Hot Reload",
            Command = hotReloadCmd,
            Shortcut = "Ctrl+Shift+F5",
            SortOrder = 200
        });
    }

    /// <summary>获取引擎 DLL 目录。</summary>
    private static string GetEngineBinariesDir()
    {
        var engineRoot = VFSService.GetAbsolutePath("/");
        if (!string.IsNullOrEmpty(engineRoot))
        {
            var candidates = new[]
            {
                Path.Combine(engineRoot, "Engine", "Build", "bin", "Debug"),
                Path.Combine(engineRoot, "Engine", "Build", "bin", "Release"),
                Path.Combine(engineRoot, "Build", "bin", "Debug"),
                Path.Combine(engineRoot, "Build", "bin", "Release"),
            };

            foreach (var candidate in candidates)
            {
                if (Directory.Exists(candidate))
                    return candidate;
            }
        }

        return AppContext.BaseDirectory;
    }
}
