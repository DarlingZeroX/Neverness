// Neverness.Runtime.Interop — 禁止 DllImport；仅通过 NNNativeAPI 函数表间接调用 Native。

using System.Runtime.InteropServices;
using Neverness.Managed.Core;

namespace Neverness.Managed.Interop;

/// <summary>
/// 将 Native 传入的 <c>NNNativeAPI*</c> 安装到进程内静态缓存，并提供 UTF-8 日志封装。
/// </summary>
public static unsafe class NativeApiBootstrap
{
	private static NNNativeApi s_api;
	private static volatile bool s_installed;

	/// <summary>是否已成功安装且版本相符。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>从 Native 指针按值复制函数表（不依赖指针生命周期）。</summary>
	public static void Install(nint nativeApiTable)
	{
		var p = (NNNativeApi*)nativeApiTable;
		if (p == null)
		{
			return;
		}

		if (p->ApiVersion != NNNativeApiConstants.ApiVersion)
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

	/// <summary>经已安装函数表写入 UTF-8 日志。</summary>
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

		Span<byte> buf = stackalloc byte[utf8Text.Length + 1];
		utf8Text.CopyTo(buf);
		buf[^1] = 0;
		fixed (byte* pb = buf)
		{
			s_api.LogInfo(pb);
		}
	}
}
