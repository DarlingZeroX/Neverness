using System.Runtime.InteropServices;

namespace VisionGal.Managed.Core;

/// <summary>
/// 与 Native <c>VGNativeAPI</c>（<c>VGManagedCore/NativeAPI.h</c>）逐字段对齐的镜像结构。
/// 调用约定：<c>LogInfo</c> 使用 stdcall 与 C++ <c>VGNativeLogInfoFn</c> 及托管 <c>UnmanagedCallersOnly</c> 一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGNativeApi
{
	public uint ApiVersion;
	public uint Reserved0;

	/// <summary>UTF-8 以 NUL 结尾的日志消息指针；语义同 Native <c>const char*</c>。</summary>
	/// <remarks>Windows x64 上默认非托管调用约定与 MSVC <c>__stdcall</c> 对单指针参数等价；显式 CallConv 语法因 SDK 差异易触发 CS8890，故采用默认 unmanaged。</remarks>
	public delegate* unmanaged<byte*, void> LogInfo;
}
