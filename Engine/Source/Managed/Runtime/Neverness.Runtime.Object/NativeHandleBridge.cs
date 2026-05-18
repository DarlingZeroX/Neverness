using System.Runtime.InteropServices;
using System.Text;
using Neverness.Managed.Engine;

namespace Neverness.Managed.Object;

/// <summary>
/// 透過 <see cref="EngineNativeApiBootstrap.EngineApi"/> 之 <c>Object</c> 子表與 Native 互操作。
/// 未安裝 ABI 或函數指標為空時回傳安全預設值，不拋出例外。
/// </summary>
public static unsafe class NativeHandleBridge
{
	/// <summary>
	/// 經 Native <c>createObject</c> 建立物件控制代碼；成功時 Native 引用計數為 1，由 <see cref="LifetimeSystem"/> 持有至 <see cref="VGObject.Dispose"/>。
	/// </summary>
	/// <param name="typeNameUtf8">物件型別名稱（UTF-8）。</param>
	/// <returns>非零表示成功；零表示失敗或未安裝 ABI。</returns>
	public static NNObjectHandle CreateObject(ReadOnlySpan<byte> typeNameUtf8)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return default;
		}

		ref readonly var api = ref EngineNativeApiBootstrap.EngineApi.Object;
		if (api.CreateObject == null)
		{
			return default;
		}

		fixed (byte* p = typeNameUtf8)
		{
			var h = api.CreateObject(p);
			return new NNObjectHandle(h);
		}
	}

	/// <summary>以 managed 字串建立 Native 物件控制代碼。</summary>
	public static NNObjectHandle CreateObject(string typeName)
	{
		ArgumentNullException.ThrowIfNull(typeName);
		return CreateObject(Encoding.UTF8.GetBytes(typeName));
	}

	/// <summary>銷毀 Native 物件控制代碼。</summary>
	public static void DestroyObject(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var destroy = EngineNativeApiBootstrap.EngineApi.Object.DestroyObject;
		if (destroy != null)
		{
			destroy(handle.Value);
		}
	}

	/// <summary>增加 Native 引用計數。</summary>
	public static void Retain(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var retain = EngineNativeApiBootstrap.EngineApi.Object.RetainObject;
		if (retain != null)
		{
			retain(handle.Value);
		}
	}

	/// <summary>減少 Native 引用計數。</summary>
	public static void Release(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var release = EngineNativeApiBootstrap.EngineApi.Object.ReleaseObject;
		if (release != null)
		{
			release(handle.Value);
		}
	}

	/// <summary>讀取 Native 引用計數；未安裝時回傳 0。</summary>
	public static uint GetRefCount(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return 0;
		}

		var getRefCount = EngineNativeApiBootstrap.EngineApi.Object.GetRefCount;
		return getRefCount != null ? getRefCount(handle.Value) : 0;
	}

	/// <summary>查詢 Native 物件是否仍存活（非零為存活）。</summary>
	public static bool IsAlive(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var isAlive = EngineNativeApiBootstrap.EngineApi.Object.IsAlive;
		return isAlive != null && isAlive(handle.Value) != 0;
	}

	/// <summary>讀取 Native 物件型別名稱至 UTF-8 緩衝區。</summary>
	public static string? TryGetTypeName(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return null;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.Object.GetTypeName;
		if (fn == null)
		{
			return null;
		}

		Span<byte> buffer = stackalloc byte[256];
		fixed (byte* p = buffer)
		{
			var written = fn(handle.Value, p, (nuint)buffer.Length);
			if (written <= 0)
			{
				return null;
			}

			return Encoding.UTF8.GetString(buffer[..written]);
		}
	}
}
