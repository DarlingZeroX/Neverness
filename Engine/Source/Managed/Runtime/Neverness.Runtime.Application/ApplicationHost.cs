// Neverness.Runtime.Application — NNApplicationAPI 托管门面；禁止在产品代码中散落 delegate*。

using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Application;

/// <summary>
/// Runtime Host 生命周期封装（SDL 子系统、事件泵、帧边界）。
/// 窗口创建请使用 <see cref="WindowHost"/>。
/// </summary>
public static unsafe class ApplicationHost
{
	/// <summary>引擎 Application 子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Application.Initialize != null;

	/// <summary>初始化 SDL（可重复调用；已初始化则成功）。</summary>
	public static bool Initialize()
	{
		if (!TryGetApplicationApi(out var api) || api.Initialize == null)
		{
			return false;
		}

		return api.Initialize();
	}

	/// <summary>泵送事件；返回 false 表示应退出主循环。</summary>
	public static bool PumpEvents()
	{
		if (!TryGetApplicationApi(out var api) || api.PumpEvents == null)
		{
			return false;
		}

		return api.PumpEvents();
	}

	/// <summary>关闭所有窗口并退出 SDL。</summary>
	public static void Shutdown()
	{
		if (!TryGetApplicationApi(out var api) || api.Shutdown == null)
		{
			return;
		}

		api.Shutdown();
	}

	public static void BeginFrame()
	{
		if (!TryGetApplicationApi(out var api) || api.BeginFrame == null)
		{
			return;
		}

		api.BeginFrame();
	}

	public static void EndFrame()
	{
		if (!TryGetApplicationApi(out var api) || api.EndFrame == null)
		{
			return;
		}

		api.EndFrame();
	}

	private static bool TryGetApplicationApi(out NNApplicationApi api)
	{
		api = default;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		api = EngineNativeApiBootstrap.EngineApi.Application;
		return true;
	}
}
