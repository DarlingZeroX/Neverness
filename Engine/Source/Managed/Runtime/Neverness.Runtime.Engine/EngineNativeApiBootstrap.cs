// Neverness.Runtime.Interop — Engine Service 函数表安装；禁止 DllImport。

using System.Runtime.InteropServices;
using Neverness.Runtime.Core;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Interop;

/// <summary>
/// 从 <c>NNNativeAPI.engineServices</c> 安装 <see cref="NNNativeEngineApi"/> 镜像（按值复制函数指针）。
/// </summary>
public static unsafe class EngineNativeApiBootstrap
{
	/// <summary>是否已成功校验 layoutVersion 并缓存子表。</summary>
	public static bool IsInstalled => EngineNativeApiCache.IsInstalled;

	/// <summary>已安装引擎服务表；未安装时为零结构。</summary>
	public static ref readonly NNNativeEngineApi EngineApi => ref EngineNativeApiCache.EngineApi;

	/// <summary>由 <c>NNNativeAPI*</c> 解析并安装 engineServices。</summary>
	public static void InstallFromNativeApiTable(nint nativeApiTable)
	{
		var p = (NNNativeApi*)nativeApiTable;
		if (p == null)
		{
			Console.WriteLine("[EngineNativeApiBootstrap] nativeApiTable 指针为空");
			return;
		}

		if (p->ApiVersion != NNNativeApiConstants.ApiVersion)
		{
			Console.WriteLine($"[EngineNativeApiBootstrap] ApiVersion 不匹配: 期望 {NNNativeApiConstants.ApiVersion}, 实际 {p->ApiVersion}");
			return;
		}

		if (p->EngineServices == 0)
		{
			Console.WriteLine("[EngineNativeApiBootstrap] EngineServices 指针为空");
			return;
		}

		var pe = (NNNativeEngineApi*)p->EngineServices;
		if (pe->LayoutVersion != NNNativeEngineApiConstants.LayoutVersion)
		{
			Console.WriteLine($"[EngineNativeApiBootstrap] LayoutVersion 不匹配: 期望 {NNNativeEngineApiConstants.LayoutVersion}, 实际 {pe->LayoutVersion}");
			return;
		}

		EngineNativeApiCache.Install(in *pe);
		Console.WriteLine("[EngineNativeApiBootstrap] 安装成功");
	}

	/// <summary>
	/// 供测试与宿主验证：经函数表演练 Stub（Timing、AsyncWait、Entity 冒烟）。
	/// 生产 Bootstrap 路径不应默认调用。
	/// </summary>
	public static void ExerciseStubInteropPath()
	{
		if (!IsInstalled)
		{
			return;
		}

		ref readonly var s_engineApi = ref EngineApi;

		if (s_engineApi.AsyncWait.CreateWait != null &&
		    s_engineApi.AsyncWait.TryComplete != null &&
		    s_engineApi.AsyncWait.ReleaseWait != null)
		{
			var h = s_engineApi.AsyncWait.CreateWait();
			_ = s_engineApi.AsyncWait.TryComplete(h);
			s_engineApi.AsyncWait.ReleaseWait(h);
		}

		if (s_engineApi.Application.Initialize != null)
		{
			_ = s_engineApi.Application.Initialize();
		}

		if (s_engineApi.Application.PumpEvents != null)
		{
			_ = s_engineApi.Application.PumpEvents();
		}
	}

	/// <summary>测试重置。</summary>
	internal static void ResetForTesting() => EngineNativeApiCache.ResetForTesting();
}
