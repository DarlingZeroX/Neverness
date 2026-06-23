// Neverness.Runtime.Interop — 禁止 DllImport；仅通过 NNNativeAPI 函数表间接调用 Native。

using System.Runtime.InteropServices;
using Neverness.Runtime.Core;

namespace Neverness.Runtime.Interop;

/// <summary>
/// 将 Native 传入的 <c>NNNativeAPI*</c> 安装到进程内静态缓存。
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

		s_api = *p;
		s_installed = true;
	}
}
