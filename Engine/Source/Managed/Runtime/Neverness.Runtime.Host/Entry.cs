using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Neverness.Runtime.Bootstrap;

namespace Neverness.Runtime.Runtime;

/// <summary>
/// 托管 Runtime 薄 UCO 入口：仅转发至 <see cref="RuntimeBootstrap"/> / <see cref="RuntimeMainLoop"/>。
/// Native 外循环每帧调用 <see cref="RuntimeTick"/>；禁止在此堆积产品演练逻辑。
/// </summary>
public static class Entry
{
	/// <summary>
	/// 安装 Native API 表并初始化托管 Kernel（非阻塞；帧推进见 <see cref="RuntimeTick"/>）。
	/// </summary>
	/// <param name="nativeApiTable">Native <c>NNNativeApi_GetDefaultTable()</c> 指针。</param>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void Bootstrap(nint nativeApiTable)
	{
		var ctx = new NativeBootstrapContext
		{
			NativeApiTable = nativeApiTable,
			RunMode = NativeBootstrapRunMode.NativeDriven,
		};
		RuntimeBootstrap.Start(in ctx);
	}

	/// <summary>
	/// 打包 API 版本：高 16 位 <c>NNNativeAPI.apiVersion</c>，低 16 位 <c>NNNativeEngineAPI.layoutVersion</c>。
	/// </summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static int GetApiVersion() => RuntimeBootstrap.GetPackedApiVersion();

	/// <summary>
	/// 托管 Kernel 单帧 Tick（Early → Fixed → Update → Late → MainThread → Render）。
	/// </summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void RuntimeTick(float deltaTimeSeconds) => RuntimeMainLoop.Tick(deltaTimeSeconds);
}
