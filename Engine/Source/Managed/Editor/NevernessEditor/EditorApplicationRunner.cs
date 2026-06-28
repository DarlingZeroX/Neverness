using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Application.Public;
using Neverness.Runtime.Bootstrap;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Engine.Runtime;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Script.Public;
// using Neverness.Editor.ImGuiFrontend.Features;
// using Neverness.Editor.ImGuiFrontend.Public;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.Assets.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Editor.MediaImporter;
using Neverness.Editor.Media;
using Neverness.Editor.Rmlui.Public;
using Neverness.Editor.Script.Public;
using Neverness.Editor.CodeEditor.Public;
using Neverness.Rendering.Core;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Audio;
using Neverness.Runtime.Audio.Native;
using Neverness.Runtime.Scene;
using Neverness.Editor.Settings;

namespace NevernessEditor;

/// <summary>
/// 编辑器前端类型。
/// </summary>
internal enum EditorFrontend
{
    /// <summary>ImGui 前端（Developer Tools）。</summary>
    ImGui,

    /// <summary>Avalonia 前端（主编辑器）。</summary>
    Avalonia
}

internal static class EditorApplicationRunner
{
	private const float FallbackDeltaSeconds = 1f / 60f;

	private static bool s_isInstalled;
	private static EditorEventPump? s_editorEventPump;
	private static AvaloniaEditorHost? s_avaloniaHost;

	/// <summary>
	/// 解析前端类型。
	/// </summary>
	private static EditorFrontend ParseFrontend(string[] args)
	{
		for (int i = 0; i < args.Length - 1; i++)
		{
			if (args[i] == "--frontend")
			{
				return args[i + 1].ToLowerInvariant() switch
				{
					"avalonia" => EditorFrontend.Avalonia,
					"imgui" => EditorFrontend.ImGui,
					_ => EditorFrontend.Avalonia // 默认使用 Avalonia
				};
			}
		}

		// 默认使用 Avalonia
		return EditorFrontend.Avalonia;
	}

    public static void Install(SdlWindow window)
    {
        s_isInstalled = true;
        var sceneManager = new SceneManager();

        /* Phase 1: 基础框架 */
        EditorFrameworkModule.Install();
        EditorCoreModule.Install(); // 注册 BuiltinMenuContributor

        /* Phase 1.5: 设置系统（必须在 AssetsModule 之前，CSharpScriptAssetOpener 依赖） */
        SettingsModule.Install();

        /* Phase 2: 业务模块（注册菜单贡献者、命令等） */
        MediaImporterModule.Install();
        MediaModule.Install();
        CoreModuleImp.Context.RegisterService<IAudioService>(new NativeAudioService());
        RmluiModule.Install();
        AssetsModule.Install(sceneManager);
        SceneModule.Install(sceneManager);
        ScriptEditorModule.Install(CoreModuleImp.Context);
        CodeEditorModule.Install();

        /* Phase 2.5: 初始化 Application 模块（创建 SdlInputProvider） */
        ApplicationModule.Initialize();

        /* Phase 3: 前端安装（所有菜单贡献者已注册） */
        InstallAvaloniaFrontend(window);
        s_avaloniaHost?.InstallModule();

        /* 注册场景子系统到 RuntimeLoop，驱动 ECS Tick */
        RuntimeInitializer.RegisterSubsystem(new SceneSubsystem(sceneManager));

        /* 模块就绪后创建编辑器事件路由器（替换 NativeEventPump） */
        s_editorEventPump = new EditorEventPump(
            window.Events,
            CoreModuleImp.Context.Events);

        /* Phase 4: 编辑器组装（ViewModel、Controller、View） */
        EditorCompositionRoot.Build();

        /* 注入 RmlUI 资产路径解析器（Editor.Assets → ViewportService） */
        InjectRmlUIAssetPathResolver();

        /* Phase 5: 前端面板内容和上下文菜单 */
        AvaloniaFrontendModule.SetDockPanelContent();
        s_avaloniaHost?.RegisterContextMenuContributors();

        /* Phase 6: 自动加载上次打开的场景 */
        AutoLoadLastScene(sceneManager);
    }


    public static int Run(EditorLaunchOptions options, string[]? args = null)
	{
		ArgumentNullException.ThrowIfNull(options);

		// 解析前端类型
		var frontend = args != null ? ParseFrontend(args) : EditorFrontend.Avalonia;
		Console.WriteLine($"[EditorApplicationRunner] 使用前端: {frontend}");

		if (!NativeApiTableLoader.TryResolve(out var nativeApiTable, out var loadError))
		{
			Console.Error.WriteLine(loadError);
			return 2;
		}

		var bootstrapContext = new NativeBootstrapContext
		{
			NativeApiTable = nativeApiTable,
			RunMode = NativeBootstrapRunMode.NativeDriven,
		};

		if (!RuntimeBootstrap.Start(in bootstrapContext))
		{
			Console.Error.WriteLine(
				"NervernessEditor: RuntimeBootstrap.Start failed.\n" +
				"Interop bootstrap or native API version negotiation did not complete.");
			return 1;
		}

		try
		{
            var windowHandle = SdlWindowManager.Create(options.WindowTitle, options.Width, options.Height);
			if (!windowHandle.IsValid)
			{
				Console.Error.WriteLine("NervernessEditor: SdlWindowManager.Create failed.");
				//nativePump.Dispose();
				return 1;
			}

			var window = SdlWindowManager.Resolve(windowHandle)!;

			// 创建渲染表面（含 Diligent 设备创建）
			var renderSurface = RenderSurfaceHost.CreateSurface(windowHandle);
			if (renderSurface == null)
			{
				Console.Error.WriteLine("NervernessEditor: RenderSurfaceHost.CreateSurface failed.");
				return 1;
			}

            // 安装编辑器（模块注册、前端安装、事件泵创建等）
            Install(window);

            while (ApplicationHost.PumpEvents())
			{
				/* 清除输入瞬态标志（Down/Up），必须在 Gameplay Tick 之前 */
				var frameDeltaTime = EngineTime.DeltaTime;
				if (frameDeltaTime <= 0f) frameDeltaTime = FallbackDeltaSeconds;

				/* 在 BeginFrame 之前消费 Native 事件 */
				s_editorEventPump?.PollAndDispatch();

				/* 如果事件路由请求退出，跳出主循环 */
				if (s_editorEventPump?.QuitRequested == true)
				{
					break;
				}

				ApplicationHost.BeginFrame();

				var deltaTime = EngineTime.DeltaTime;
				if (deltaTime <= 0f)
				{
					deltaTime = FallbackDeltaSeconds;
				}

				RuntimeMainLoop.Tick(deltaTime);

				// 主线程渲染调度（Diligent immediate context 非线程安全）
				RenderingLoop.TickRendering();

				AssetsModule.Tick();

				/* 刷新延迟事件 */
				s_editorEventPump?.FlushDeferred();

				ApplicationHost.EndFrame();

                ApplicationModule.EndFrame(frameDeltaTime);
            }

			return 0;
		}
		finally
		{
			s_editorEventPump?.Dispose();
			s_editorEventPump = null;

			/* 清理 Avalonia 宿主 */
			s_avaloniaHost?.Dispose();
			s_avaloniaHost = null;

			/* 清理 CompositionRoot */
			EditorCompositionRoot.Shutdown();

			ApplicationModule.Shutdown();
			ApplicationHost.Shutdown();
			RuntimeBootstrap.Shutdown();
		}
	}

	/// <summary>
	/// 注入 RmlUI 资产路径解析器——将 EditorAssetDatabase.TryGetPath 桥接到 ViewportService。
	/// 避免 Editor.Scene 直接依赖 Editor.Assets。
	/// </summary>
	private static void InjectRmlUIAssetPathResolver()
	{
		var viewportService = CoreModuleImp.Context.GetService<IViewportService>();
		if (viewportService == null)
		{
			Console.WriteLine("[EditorApplicationRunner] 注入 RmlUI 资产路径解析器失败: IViewportService 未注册");
			return;
		}

		viewportService.AssetPathResolver = guid =>
		{
			var nGuid = new GUID(guid.High, guid.Low);
			if (Neverness.Editor.Assets.EditorAssetDatabase.TryGetPath(nGuid, out var virtualPath))
				return virtualPath.FullPath;
			return null;
		};

		Console.WriteLine("[EditorApplicationRunner] RmlUI 资产路径解析器已注入");
	}

	/// <summary>
	/// 自动加载上次打开的场景（从 EditorSettings.Session 读取）。
	/// </summary>
	private static void AutoLoadLastScene(SceneManager sceneManager)
	{
		var lastScene = EditorSettings.Session.LastOpenedScene;
		if (string.IsNullOrEmpty(lastScene))
		{
			Console.WriteLine("[EditorApplicationRunner] 无上次场景记录，跳过自动加载。");
			return;
		}

		Console.WriteLine($"[EditorApplicationRunner] 自动加载上次场景: {lastScene}");
		var sceneName = System.IO.Path.GetFileNameWithoutExtension(lastScene);
		var result = sceneManager.LoadSceneFromAsset(sceneName, lastScene);
		if (!result)
		{
			Console.WriteLine($"[EditorApplicationRunner] 自动加载失败（场景文件可能已删除）: {lastScene}");
			// 清除无效记录
			EditorSettings.Session.LastOpenedScene = null;
		}
	}

	/// <summary>
	/// 安装 Avalonia 前端。
	/// </summary>
	private static void InstallAvaloniaFrontend(SdlWindow window)
	{
		Console.WriteLine("[EditorApplicationRunner] 安装 Avalonia 前端...");

		s_avaloniaHost = new AvaloniaEditorHost();
		s_avaloniaHost.Start(window);
		s_avaloniaHost.InstallShell(window);

		Console.WriteLine("[EditorApplicationRunner] Avalonia 前端已安装");
	}
}
