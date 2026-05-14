using System.Runtime.InteropServices;

namespace VisionGal.Managed.Core;

/// <summary>
/// 将 Native 传入的 <c>VGNativeAPI*</c> 安装到进程内静态缓存，并提供类型安全的 UTF-8 日志封装。
/// Phase 2：禁止在此使用 <c>DllImport</c> 调引擎；一律通过函数表间接调用。
/// </summary>
public static unsafe class NativeApiBootstrap
{
	private static VGNativeApi s_api;
	private static volatile bool s_installed;

	/// <summary>是否已成功安装且版本匹配（供托管侧自检）。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>
	/// 从 native 传入的指针复制一份 API 表到托管静态存储（按值复制函数指针，避免依赖指针生命周期）。
	/// </summary>
	/// <param name="nativeApiTable">Native <c>const VGNativeAPI*</c> 转为 <see cref="nint"/>。</param>
	public static void Install(nint nativeApiTable)
	{
		var p = (VGNativeApi*)nativeApiTable;
		if (p == null)
		{
			return;
		}

		if (p->ApiVersion != VGNativeApiConstants.ApiVersion)
		{
			return;
		}

		if (p->LogInfo == null)
		{
			return;
		}

		s_api = *p;
		s_installed = true;
	}

	/// <summary>向 Native 日志通道写入一段 UTF-8 文本（无需 NUL 结尾 span，内部临时补零）。</summary>
	public static void LogInfoUtf8(ReadOnlySpan<byte> utf8Text)
	{
		if (!s_installed || s_api.LogInfo == null)
		{
			return;
		}

		if (utf8Text.IsEmpty)
		{
			ReadOnlySpan<byte> empty = [(byte)'\0'];
			fixed (byte* pb = empty)
			{
				s_api.LogInfo(pb);
			}

			return;
		}

		// 栈上缓冲：Phase 2 仅用于短 smoke 消息；后续可换 native 侧显式长度参数或共享缓冲契约。
		Span<byte> buf = stackalloc byte[utf8Text.Length + 1];
		utf8Text.CopyTo(buf);
		buf[^1] = 0;
		fixed (byte* pb = buf)
		{
			s_api.LogInfo(pb);
		}
	}
}
