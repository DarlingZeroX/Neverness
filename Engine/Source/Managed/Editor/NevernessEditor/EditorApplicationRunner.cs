using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Public;
using Neverness.Runtime.Bootstrap;
using Neverness.Runtime.Engine.Runtime;
using Neverness.Editor.Assets.Public;

namespace NevernessEditor;

internal static class EditorApplicationRunner
{
	private const float FallbackDeltaSeconds = 1f / 60f;

	private static bool s_isInstalled;

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

			Window? window = Window.Create(options.WindowTitle, options.Width, options.Height);
			if (window is null)
			{
				Console.Error.WriteLine("NervernessEditor: Window.Create failed.");
				return 1;
			}

			while (ApplicationHost.PumpEvents())
			{
				ApplicationHost.BeginFrame();

				if (!s_isInstalled)
				{
					s_isInstalled = true;
					EditorFrameworkModule.Install(window);
					AssetsModule.Install();
				}

				var deltaTime = EngineTime.DeltaTime;
				if (deltaTime <= 0f)
				{
					deltaTime = FallbackDeltaSeconds;
				}

				RuntimeMainLoop.Tick(deltaTime);
				EditorFrameworkModule.TickEditorUI();
				ApplicationHost.EndFrame();
			}

			return 0;
		}
		finally
		{
			ApplicationHost.Shutdown();
			RuntimeBootstrap.Shutdown();
		}
	}
}
