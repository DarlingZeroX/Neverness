using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using VisionGal.Managed.Core;
using VisionGal.Managed.Engine;

namespace VisionGal.Managed.Runtime;

/// <summary>Phase 1/2：供 native 通过 <c>load_assembly_and_get_function_pointer</c> 调用的 UCO 导出入口。</summary>
public static class Entry
{
	public static volatile bool SmokeCalled;

	/// <summary>Phase 2：在 <see cref="BootstrapNativeApi"/> 成功安装 API 表并调用 <c>LogInfo</c> 后置为 true。</summary>
	public static volatile bool BootstrapNativeApiCompleted;

	/// <summary>Phase 3：引擎服務表已安裝且 <see cref="EngineNativeApiBootstrap.ExerciseStubInteropPath"/> 已執行。</summary>
	public static volatile bool BootstrapEngineInteropCompleted;

	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void Smoke()
	{
		SmokeCalled = true;
	}

	/// <summary>
	/// Phase 2：接收 Native 传入的 <c>const VGNativeAPI*</c>（以 <see cref="nint"/> 传递），安装函数表并触发一次经 ABI 的 <c>LogInfo</c>。
	/// </summary>
	/// <param name="nativeApiTable">Native 侧 <c>VGNativeApi_GetDefaultTable()</c> 返回的指针值。</param>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void BootstrapNativeApi(nint nativeApiTable)
	{
		NativeApiBootstrap.Install(nativeApiTable);
		NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapNativeApi -> Native LogInfo (Phase2 ABI)"u8);
		BootstrapNativeApiCompleted = NativeApiBootstrap.IsInstalled;

		EngineNativeApiBootstrap.InstallFromNativeApiTable(nativeApiTable);
		EngineNativeApiBootstrap.ExerciseStubInteropPath();
		BootstrapEngineInteropCompleted = EngineNativeApiBootstrap.IsInstalled;
	}
}
