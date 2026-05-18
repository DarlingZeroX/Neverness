using System.Runtime.InteropServices;

namespace Neverness.Managed.Core;

/// <summary>
/// 與 Native <c>NNNativeAPI</c>（<c>Runtime/NNRuntimeManaged/Include/NativeAPI.h</c>）逐欄位對齊的鏡像結構。
/// 呼叫約定：<c>LogInfo</c> 使用 stdcall 與 C++ <c>NNNativeLogInfoFn</c> 及託管 <c>UnmanagedCallersOnly</c> 一致。
/// Phase 3：追加 <c>EngineServices</c> 指標，指向 <c>NNNativeEngineAPI</c>（定義於 Neverness.Managed.Engine）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeApi
{
	public uint ApiVersion;
	public uint Reserved0;

	/// <summary>UTF-8 以 NUL 結尾的日誌訊息指標；語意同 Native <c>const char*</c>。</summary>
	public delegate* unmanaged<byte*, void> LogInfo;

	/// <summary>指向 Native <c>const NNNativeEngineAPI*</c>；可為 0 表示未掛載（預設表應非 0）。</summary>
	public nint EngineServices;
}
