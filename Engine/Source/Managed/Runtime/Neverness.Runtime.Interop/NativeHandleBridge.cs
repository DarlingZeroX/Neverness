// Neverness.Runtime.Interop — Native Object 子表薄封装；对齐 NNNativeEngineAPI.object。

using System.Runtime.InteropServices;
using System.Text;
using Neverness.Managed.Engine;

namespace Neverness.Managed.Interop;

/// <summary>
/// 经 <see cref="EngineNativeApiBootstrap.EngineApi"/> 的 Object 子表与 Native 互操作。
/// </summary>
public static unsafe class NativeHandleBridge
{
	/// <summary>经 Native createObject 建立控制代码。</summary>
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

	/// <summary>以 managed 字符串建立 Native 对象。</summary>
	public static NNObjectHandle CreateObject(string typeName)
	{
		ArgumentNullException.ThrowIfNull(typeName);
		return CreateObject(Encoding.UTF8.GetBytes(typeName));
	}

	/// <summary>销毁 Native 对象控制代码。</summary>
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

	/// <summary>增加 Native 引用计数。</summary>
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

	/// <summary>减少 Native 引用计数。</summary>
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

	/// <summary>读取 Native 引用计数。</summary>
	public static uint GetRefCount(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return 0;
		}

		var getRefCount = EngineNativeApiBootstrap.EngineApi.Object.GetRefCount;
		return getRefCount != null ? getRefCount(handle.Value) : 0;
	}

	/// <summary>查询 Native 对象是否仍存活。</summary>
	public static bool IsAlive(NNObjectHandle handle)
	{
		if (handle.Value == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var isAlive = EngineNativeApiBootstrap.EngineApi.Object.IsAlive;
		return isAlive != null && isAlive(handle.Value) != 0;
	}

	/// <summary>读取 Native 对象类型名。</summary>
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
