using System.Runtime.InteropServices;
using System.Text;
using Neverness.Managed.Engine;

namespace Neverness.Managed.Assets;

/// <summary>
/// 託管資產登記表薄封裝；優先經 <see cref="EngineNativeApiBootstrap.EngineApi.AssetRegistry"/> 與 Native 同步。
/// 未安裝 ABI 時回退至進程內字典（Stub / 單元測試）。
/// </summary>
public static class AssetDatabase
{
	private static readonly Dictionary<string, GUID> s_pathToGuid = new(StringComparer.OrdinalIgnoreCase);
	private static readonly Dictionary<string, string> s_guidToPath = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>已登記資產數量（託管快取）。</summary>
	public static int Count => s_pathToGuid.Count;

	/// <summary>登記虛擬路徑與 GUID。</summary>
	public static bool Register(string virtualPath, GUID guid)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(virtualPath);
		if (guid.IsZero)
		{
			return false;
		}

		if (TryRegisterNative(virtualPath, guid))
		{
			Cache(virtualPath, guid);
			return true;
		}

		if (s_pathToGuid.ContainsKey(virtualPath))
		{
			return false;
		}

		Cache(virtualPath, guid);
		return true;
	}

	/// <summary>依路徑解析 GUID。</summary>
	public static bool TryResolveGuid(string virtualPath, out GUID guid)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(virtualPath);
		if (TryResolveGuidNative(virtualPath, out guid))
		{
			return true;
		}

		return s_pathToGuid.TryGetValue(virtualPath, out guid);
	}

	/// <summary>依 GUID 解析虛擬路徑。</summary>
	public static bool TryResolvePath(GUID guid, out string? virtualPath)
	{
		if (guid.IsZero)
		{
			virtualPath = null;
			return false;
		}

		if (TryResolvePathNative(guid, out virtualPath))
		{
			return true;
		}

		return s_guidToPath.TryGetValue(guid.ToHexString(), out virtualPath);
	}

	private static void Cache(string virtualPath, GUID guid)
	{
		s_pathToGuid[virtualPath] = guid;
		s_guidToPath[guid.ToHexString()] = virtualPath;
	}

	private static unsafe bool TryRegisterNative(string virtualPath, GUID guid)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.RegisterAsset;
		if (fn == null)
		{
			return false;
		}

		var bytes = Encoding.UTF8.GetBytes(virtualPath);
		fixed (byte* p = bytes)
		{
			return fn(p, guid.ToNative()) == 0;
		}
	}

	private static unsafe bool TryResolveGuidNative(string virtualPath, out GUID guid)
	{
		guid = GUID.Zero;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.ResolveGuidByPath;
		if (fn == null)
		{
			return false;
		}

		var bytes = Encoding.UTF8.GetBytes(virtualPath);
		NNGuid native;
		fixed (byte* p = bytes)
		{
			if (fn(p, &native) != 0)
			{
				return false;
			}
		}

		guid = GUID.FromNative(native);
		return !guid.IsZero;
	}

	private static unsafe bool TryResolvePathNative(GUID guid, out string? virtualPath)
	{
		virtualPath = null;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.ResolvePathByGuid;
		if (fn == null)
		{
			return false;
		}

		Span<byte> buffer = stackalloc byte[512];
		fixed (byte* p = buffer)
		{
			var written = fn(guid.ToNative(), p, (nuint)buffer.Length);
			if (written <= 0)
			{
				return false;
			}

			virtualPath = Encoding.UTF8.GetString(buffer[..written]);
			return true;
		}
	}

	/// <summary>清空託管快取（Bootstrap 與單元測試重置；不呼叫 Native 登記表）。</summary>
	public static void ClearForTesting()
	{
		s_pathToGuid.Clear();
		s_guidToPath.Clear();
	}
}
