// Neverness.Runtime.Scene — 经 NNSceneAPI 访问 Native 场景图；场景细节在 C++ Kernel。

using System.Runtime.InteropServices;
using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Scene;

/// <summary>
/// <see cref="NNSceneApi"/> 薄封装：Managed 不持有场景存储，仅经 ABI 转发。
/// </summary>
public static unsafe class SceneNativeBridge
{
	/// <summary>引擎 Scene 子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Scene.Spawn != null;

	/// <summary>句柄是否非零（不保证 Native 槽位仍有效，销毁后勿复用）。</summary>
	public static bool IsEntityAlive(NNEntityHandle handle) => handle.Value != 0;

	/// <summary>加载命名场景；成功返回非零。</summary>
	public static int LoadScene(string sceneName)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(sceneName);
		if (!TryGetSceneApi(out var api) || api.LoadScene == null)
		{
			return 0;
		}

		var utf8 = Encoding.UTF8.GetBytes(sceneName);
		fixed (byte* p = utf8)
		{
			return api.LoadScene(p);
		}
	}

	/// <summary>卸载命名场景。</summary>
	public static int UnloadScene(string sceneName)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(sceneName);
		if (!TryGetSceneApi(out var api) || api.UnloadScene == null)
		{
			return 0;
		}

		var utf8 = Encoding.UTF8.GetBytes(sceneName);
		fixed (byte* p = utf8)
		{
			return api.UnloadScene(p);
		}
	}

	/// <summary>由 Prefab 虚拟路径生成实体；失败返回零句柄。</summary>
	public static NNEntityHandle Spawn(string prefabVirtualPathUtf8)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(prefabVirtualPathUtf8);
		if (!TryGetSceneApi(out var api) || api.Spawn == null)
		{
			return default;
		}

		var utf8 = Encoding.UTF8.GetBytes(prefabVirtualPathUtf8);
		fixed (byte* p = utf8)
		{
			return new NNEntityHandle(api.Spawn(p));
		}
	}

	/// <summary>销毁场景实体。</summary>
	public static void Destroy(NNEntityHandle entity)
	{
		if (entity.Value == 0 || !TryGetSceneApi(out var api) || api.Destroy == null)
		{
			return;
		}

		api.Destroy(entity.Value);
	}

	/// <summary>按名称查找实体。</summary>
	public static NNEntityHandle Find(string entityNameUtf8)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(entityNameUtf8);
		if (!TryGetSceneApi(out var api) || api.Find == null)
		{
			return default;
		}

		var utf8 = Encoding.UTF8.GetBytes(entityNameUtf8);
		fixed (byte* p = utf8)
		{
			return new NNEntityHandle(api.Find(p));
		}
	}

	/// <summary>设置实体显示名（UTF-8）；成功返回非零。</summary>
	public static int SetEntityName(NNEntityHandle entity, string nameUtf8)
	{
		if (entity.Value == 0)
		{
			return 0;
		}

		if (!TryGetSceneApi(out var api) || api.SetEntityName == null)
		{
			return 0;
		}

		var utf8 = Encoding.UTF8.GetBytes(nameUtf8);
		fixed (byte* p = utf8)
		{
			return api.SetEntityName(entity.Value, p);
		}
	}

	/// <summary>读取实体名称；失败或缓冲区不足时返回 null。</summary>
	public static string? TryGetEntityName(NNEntityHandle entity, int maxBytes = 256)
	{
		if (entity.Value == 0 || !TryGetSceneApi(out var api) || api.GetEntityName == null)
		{
			return null;
		}

		Span<byte> buffer = stackalloc byte[maxBytes];
		fixed (byte* p = buffer)
		{
			if (api.GetEntityName(entity.Value, p, (nuint)buffer.Length) <= 0)
			{
				return null;
			}
		}

		var len = buffer.IndexOf((byte)0);
		if (len < 0)
		{
			len = buffer.Length;
		}

		return Encoding.UTF8.GetString(buffer[..len]);
	}

	private static bool TryGetSceneApi(out NNSceneApi api)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			api = default;
			return false;
		}

		api = EngineNativeApiBootstrap.EngineApi.Scene;
		return true;
	}
}
