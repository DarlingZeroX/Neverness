using System.Runtime.InteropServices;

namespace Neverness.Runtime.Core;

/// <summary>
/// 与 Native <c>NNNativeAPI</c>（<c>Runtime/NNRuntimeManaged/Include/NativeAPI.h</c>）逐字段对齐的镜像结构。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeApi
{
	public uint ApiVersion;
	public uint Reserved0;

	/// <summary>指向 Native <c>const NNNativeEngineAPI*</c>；可为 0 表示未挂载（默认表应非 0）。</summary>
	public nint EngineServices;
}
