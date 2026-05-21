using System.Runtime.InteropServices;

namespace Neverness.Runtime.Core;

/// <summary>
/// 与 Native <c>NNNativeAPI</c>（<c>Runtime/NNRuntimeManaged/Include/NativeAPI.h</c>）逐字段对齐的镜像结构。
/// 调用约定：<c>LogInfo</c> 使用 stdcall 与 C++ <c>NNNativeLogInfoFn</c> 及托管 <c>UnmanagedCallersOnly</c> 一致。
/// Phase 3：追加 <c>EngineServices</c> 指针，指向 <c>NNNativeEngineAPI</c>（定义于 Neverness.Managed.Engine）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeApi
{
	public uint ApiVersion;
	public uint Reserved0;

	/// <summary>UTF-8 以 NUL 结尾的日志消息指针；语义同 Native <c>const char*</c>。</summary>
	public delegate* unmanaged<byte*, void> LogInfo;

	/// <summary>指向 Native <c>const NNNativeEngineAPI*</c>；可为 0 表示未挂载（默认表应非 0）。</summary>
	public nint EngineServices;
}
