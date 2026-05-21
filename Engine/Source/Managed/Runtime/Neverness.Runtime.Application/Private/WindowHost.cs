// Neverness.Runtime.Application — NNWindowAPI 托管门面；禁止在产品代码中散落 delegate*。

using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// Runtime 窗口子系统封装（多窗口、Native 句柄）；与 <see cref="ApplicationHost"/> 生命周期解耦。
/// </summary>
public static unsafe class WindowHost
{
	/// <summary>无效句柄常量。</summary>
	public static NNWindowHandle Invalid => NNWindowHandle.Invalid;

	/// <summary>窗口子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Window.Create != null;

	/// <summary>
	/// 创建窗口；失败返回 <see cref="Invalid"/>。
	/// 调用前须已成功 <see cref="ApplicationHost.Initialize"/>。
	/// </summary>
	public static NNWindowHandle Create(string title, int width, int height, bool resizable = true, bool maximized = false, bool hidden = false)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(title);
		if (!TryGetWindowApi(out var api) || api.Create == null)
		{
			return Invalid;
		}

		var utf8 = Encoding.UTF8.GetBytes(title);
		fixed (byte* titlePtr = utf8)
		{
			var desc = new NNWindowDesc
			{
				Title = titlePtr,
				Width = width,
				Height = height,
				Resizable = resizable,
				Maximized = maximized,
				Hidden = hidden,
			};

			var handle = api.Create(&desc);
			return handle == 0 ? Invalid : new NNWindowHandle(handle);
		}
	}

	/// <summary>销毁窗口；无效句柄为 no-op。</summary>
	public static void Destroy(NNWindowHandle handle)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.Destroy == null)
		{
			return;
		}

		api.Destroy(handle.Value);
	}

	/// <summary>设置窗口标题。</summary>
	public static void SetTitle(NNWindowHandle handle, string title)
	{
		if (!handle.IsValid || string.IsNullOrEmpty(title))
		{
			return;
		}

		if (!TryGetWindowApi(out var api) || api.SetTitle == null)
		{
			return;
		}

		var utf8 = Encoding.UTF8.GetBytes(title);
		fixed (byte* p = utf8)
		{
			api.SetTitle(handle.Value, p);
		}
	}

	public static void SetSize(NNWindowHandle handle, int width, int height)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.SetSize == null)
		{
			return;
		}

		api.SetSize(handle.Value, width, height);
	}

	public static (int Width, int Height) GetSize(NNWindowHandle handle)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.GetSize == null)
		{
			return (0, 0);
		}

		int w = 0, h = 0;
		api.GetSize(handle.Value, &w, &h);
		return (w, h);
	}

	public static void SetPosition(NNWindowHandle handle, int x, int y)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.SetPosition == null)
		{
			return;
		}

		api.SetPosition(handle.Value, x, y);
	}

	public static (int X, int Y) GetPosition(NNWindowHandle handle)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.GetPosition == null)
		{
			return (0, 0);
		}

		int x = 0, y = 0;
		api.GetPosition(handle.Value, &x, &y);
		return (x, y);
	}

	public static void SetResizable(NNWindowHandle handle, bool value)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.SetResizable == null)
		{
			return;
		}

		api.SetResizable(handle.Value, value);
	}

	public static void Maximize(NNWindowHandle handle)
	{
		if (handle.IsValid && TryGetWindowApi(out var api) && api.Maximize != null)
		{
			api.Maximize(handle.Value);
		}
	}

	public static void Minimize(NNWindowHandle handle)
	{
		if (handle.IsValid && TryGetWindowApi(out var api) && api.Minimize != null)
		{
			api.Minimize(handle.Value);
		}
	}

	public static void Restore(NNWindowHandle handle)
	{
		if (handle.IsValid && TryGetWindowApi(out var api) && api.Restore != null)
		{
			api.Restore(handle.Value);
		}
	}

	public static void Show(NNWindowHandle handle)
	{
		if (handle.IsValid && TryGetWindowApi(out var api) && api.Show != null)
		{
			api.Show(handle.Value);
		}
	}

	public static void Hide(NNWindowHandle handle)
	{
		if (handle.IsValid && TryGetWindowApi(out var api) && api.Hide != null)
		{
			api.Hide(handle.Value);
		}
	}

	/// <summary>
	/// 获取平台原生窗口指针（HWND / NSWindow / X11），供 Vulkan / DX12 / RmlUi 使用。
	/// </summary>
	public static nint GetNativeHandle(NNWindowHandle handle)
	{
		if (!handle.IsValid || !TryGetWindowApi(out var api) || api.GetNativeHandle == null)
		{
			return 0;
		}

		return (nint)api.GetNativeHandle(handle.Value);
	}

	private static bool TryGetWindowApi(out NNWindowApi api)
	{
		api = default;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		api = EngineNativeApiBootstrap.EngineApi.Window;
		return true;
	}
}
