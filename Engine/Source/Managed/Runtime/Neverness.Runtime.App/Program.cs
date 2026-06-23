// Neverness.Runtime.App — 纯托管 Headless 调试宿主（不进入默认 CMake 产品目标）。

using Neverness.Runtime.Engine;
using Neverness.Runtime.Bootstrap;

namespace Neverness.Runtime.Runtime.App;

internal static class Program
{
	/// <summary>Headless 入口：无 Native 表时仅验证托管外循环可启动（Interop 未安装则立即退出）。</summary>
	public static int Main(string[] args)
	{
		_ = args;
		// 无 Native 进程时 nativeApiTable=0；用于验证 Bootstrap 外循环骨架，不替代集成测试。
		var ctx = new NativeBootstrapContext
		{
			NativeApiTable = 0,
			RunMode = NativeBootstrapRunMode.ManagedOuterLoop,
		};

		if (ctx.NativeApiTable == 0)
		{
			Console.WriteLine("Neverness.Runtime.App: 无 Native API 表，跳过（请通过 Native 宿主或集成测试验证 Bootstrap）。");
			return 0;
		}

		if (!RuntimeBootstrap.Start(in ctx))
		{
			return 1;
		}

		return 0;
	}
}
