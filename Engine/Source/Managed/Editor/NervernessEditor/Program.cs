// NervernessEditor — 统一 Runtime 入口参考（需 Native 进程提供 NNNativeApi 表并加载 UCO）。

using Neverness.Managed.Application;
using Neverness.Managed.Bootstrap;
using Neverness.Managed.Engine.Runtime;

namespace NervernessEditor;

internal static class Program
{
	/// <summary>
	/// 参考入口：Native 外循环负责 <c>NNEngineRuntimeHost_Tick</c> 时，托管仅经 <see cref="RuntimeMainLoop.Tick"/> 推进。
	/// 纯托管外循环时须由 Native <c>NNApplicationAPI</c> 内聚双 Tick，或保持 Native 宿主为主路径。
	/// </summary>
	public static int Main(string[] args)
	{
		_ = args;

		// Native 宿主注入；无表时无法启动。
		nint nativeApiTable = 0;
		if (nativeApiTable == 0)
		{
			Console.WriteLine(
				"NervernessEditor: 无 Native API 表。请通过 VGEditor（NEVERNESS_USE_RUNTIME_KERNEL）或集成宿主启动。");
			return 0;
		}

		var ctx = new NativeBootstrapContext
		{
			NativeApiTable = nativeApiTable,
			RunMode = NativeBootstrapRunMode.NativeDriven,
		};

		if (!RuntimeBootstrap.Start(in ctx))
		{
			return 1;
		}

		if (!ApplicationHost.Initialize())
		{
			return 1;
		}

		if (!ApplicationHost.OpenWindow("Neverness Editor", 1280, 720))
		{
			ApplicationHost.Shutdown();
			return 1;
		}

		while (ApplicationHost.PumpEvents())
		{
			var dt = EngineTime.DeltaTime;
			if (dt <= 0f)
			{
				dt = 1f / 60f;
			}

			RuntimeMainLoop.Tick(dt);
		}

		ApplicationHost.Shutdown();
		return 0;
	}
}
