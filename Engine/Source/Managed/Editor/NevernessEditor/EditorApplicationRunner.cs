using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Public;
using Neverness.Runtime.Bootstrap;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Engine.Runtime;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Assets.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Runtime.Scene;

namespace NevernessEditor;

internal static class EditorApplicationRunner
{
	private const float FallbackDeltaSeconds = 1f / 60f;

	private static bool s_isInstalled;
	private static EditorEventPump? s_editorEventPump;

	public static int Run(EditorLaunchOptions options)
	{
		ArgumentNullException.ThrowIfNull(options);

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
			if (!ApplicationHost.IsAvailable)
			{
				Console.Error.WriteLine(
					"NervernessEditor: Native Application API is unavailable.\n" +
					"Please confirm NevernessRuntime-Managed.dll was built with NEVERNESS_USE_ENGINE_RUNTIME_SERVICES and SDL3.dll can be loaded.");
				return 1;
			}

			if (!ApplicationHost.Initialize())
			{
				Console.Error.WriteLine(
					"NervernessEditor: ApplicationHost.Initialize failed.\n" +
					"Check the native log output above. Common causes are SDL initialization failure or an invalid startup project path.");
				return 1;
			}

			/* Native 事件泵：RuntimeBootstrap 之后即可创建 */
			var nativePump = new NativeEventPump();

			Window? window = Window.Create(options.WindowTitle, options.Width, options.Height);
			if (window is null)
			{
				Console.Error.WriteLine("NervernessEditor: Window.Create failed.");
				nativePump.Dispose();
				return 1;
			}

			while (ApplicationHost.PumpEvents())
			{
				/* 在 BeginFrame 之前消费 Native 事件 */
				s_editorEventPump?.PollAndDispatch();

				/* 如果事件路由请求退出，跳出主循环 */
				if (s_editorEventPump?.QuitRequested == true)
				{
					break;
				}

				ApplicationHost.BeginFrame();

				if (!s_isInstalled)
				{
					s_isInstalled = true;
					var sceneManager = new SceneManager();

					EditorFrameworkModule.Install(window);
					EditorCoreModule.Install();
					AssetsModule.Install(sceneManager);
					SceneModule.Install(sceneManager);

					/* 模块就绪后创建编辑器事件路由器 */
					s_editorEventPump = new EditorEventPump(
						nativePump,
						CoreModuleImp.Context.Events);
				}

				var deltaTime = EngineTime.DeltaTime;
				if (deltaTime <= 0f)
				{
					deltaTime = FallbackDeltaSeconds;
				}

				RuntimeMainLoop.Tick(deltaTime);
				EditorFrameworkModule.TickEditorUI();
				AssetsModule.Tick();

				/* 刷新延迟事件 */
				s_editorEventPump?.FlushDeferred();

				ApplicationHost.EndFrame();
			}

			return 0;
		}
		finally
		{
			s_editorEventPump?.Dispose();
			s_editorEventPump = null;

			ApplicationHost.Shutdown();
			RuntimeBootstrap.Shutdown();
		}
	}
}
