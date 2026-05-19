// Neverness.Runtime.Application — NNApplicationAPI 托管门面；禁止在产品代码中散落 delegate*。

using System.Text;
using Neverness.Managed.Engine;
using Neverness.Managed.Interop;

namespace Neverness.Managed.Application;

/// <summary>
/// 统一 Runtime Application 层封装（SDL 生命周期、主窗口、事件泵）。
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

	/// <summary>打开主窗口。</summary>
	public static bool OpenWindow(string title, int width, int height)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(title);
		if (!TryGetApplicationApi(out var api) || api.OpenWindow == null)
		{
			return false;
		}

		var utf8 = Encoding.UTF8.GetBytes(title);
		fixed (byte* p = utf8)
		{
			return api.OpenWindow(p, width, height);
		}
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

	/// <summary>关闭窗口并退出 SDL。</summary>
	public static void Shutdown()
	{
		if (!TryGetApplicationApi(out var api) || api.Shutdown == null)
		{
			return;
		}

		api.Shutdown();
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
