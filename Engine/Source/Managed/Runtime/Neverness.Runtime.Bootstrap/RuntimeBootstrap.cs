// Neverness.Runtime.Bootstrap — 进程级托管 Runtime 启动/关闭门面（Runtime 主导权迁移 Phase 1）。

using Neverness.Runtime.Application;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Bootstrap;

/// <summary>
/// 托管 Runtime 启动入口：对接 <c>Entry.Bootstrap</c>（非阻塞）或 <see cref="Neverness.Runtime.App"/> 外循环。
/// </summary>
public static class RuntimeBootstrap
{
	private static volatile bool s_running;
	private static volatile bool s_quitRequested;

	/// <summary>是否已通过 <see cref="Start"/> 成功初始化。</summary>
	public static bool IsInitialized => RuntimeInitializer.IsInitialized && ApplicationHost.IsAvailable;

	/// <summary>托管外循环是否仍在运行。</summary>
	public static bool IsRunning => s_running;

	/// <summary>
	/// 启动托管 Runtime：安装 Interop、注册子系统；<see cref="NativeBootstrapRunMode.NativeDriven"/> 下立即返回。
	/// 初始化顺序：RuntimeInitializer（Native API 表） → ApplicationHost（SDL3 + VFS）。
	/// </summary>
	public static bool Start(in NativeBootstrapContext ctx)
	{
		s_quitRequested = false;

		// 1. 安装 Native API 表（ApplicationHost 依赖此步骤）
		if (!RuntimeInitializer.Initialize(ctx.NativeApiTable))
		{
			s_running = false;
			return false;
		}

		// 2. 初始化 ApplicationHost（SDL3 + VFS + 窗口管理）
		if (!ApplicationHost.Initialize())
		{
			Console.Error.WriteLine(
                "[RuntimeBootstrap] ApplicationHost.Initialize failed. 初始化失败" +
                "Check the native log output above. Common causes are SDL initialization failure or an invalid startup project path."
                );
            s_running = false;
			return false;
		}

        if (!ApplicationHost.IsAvailable)
        {
            Console.Error.WriteLine(
                "NervernessEditor: Native Application API is unavailable.\n" +
                "Please confirm NevernessRuntime-Managed.dll was built with NEVERNESS_USE_ENGINE_RUNTIME_SERVICES and SDL3.dll can be loaded.");
            s_running = false;
            return false;
        }

        s_running = true;

		if (ctx.RunMode == NativeBootstrapRunMode.ManagedOuterLoop)
		{
			RunManagedOuterLoop();
		}

		return true;
	}

	/// <summary>请求退出；外循环模式下下一帧退出。</summary>
	public static void RequestQuit() => s_quitRequested = true;

	/// <summary>关闭子系统并清除运行状态。</summary>
	public static void Shutdown()
	{
		ApplicationHost.Shutdown();
		RuntimeInitializer.Shutdown();
		s_running = false;
		s_quitRequested = false;
	}

	/// <summary>打包 API 版本供 Native 校验：高 16 位 ApiVersion，低 16 位 LayoutVersion。</summary>
	public static int GetPackedApiVersion() => RuntimeVersionInfo.GetPackedApiVersion();

	private static void RunManagedOuterLoop()
	{
		const float targetDelta = 1f / 60f;
		var sw = System.Diagnostics.Stopwatch.StartNew();
		double accum = 0;
		while (s_running && !s_quitRequested)
		{
			sw.Stop();
			var elapsed = (float)sw.Elapsed.TotalSeconds;
			sw.Restart();
			accum += elapsed;

			// 泵送 SDL3 事件（窗口、输入、退出等）
			if (!ApplicationHost.PumpEvents())
			{
				break;
			}

			while (accum >= targetDelta && !s_quitRequested)
			{
				RuntimeMainLoop.Tick(targetDelta);
				accum -= targetDelta;
			}

			if (accum > 0.25)
			{
				accum = 0;
			}

			Thread.Sleep(1);
		}

		Shutdown();
	}
}
