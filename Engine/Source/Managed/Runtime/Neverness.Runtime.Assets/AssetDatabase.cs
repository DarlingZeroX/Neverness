using System.Runtime.InteropServices;
using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 託管資產登記表薄封裝；優先經 <see cref="EngineNativeApiBootstrap.EngineApi.AssetRegistry"/> 與 Native 同步。
/// Release 僅經 Native；<c>DEBUG</c> 在未安裝 ABI 時允許進程內字典（單元測試）。
/// </summary>
public static class AssetDatabase
{
	private static readonly Dictionary<NVirtualPath, GUID> s_pathToGuid = new();
	private static readonly Dictionary<string, NVirtualPath> s_guidToPath = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>已登記資產數量（託管快取）。</summary>
	public static int Count => s_pathToGuid.Count;

	/// <summary>登記虛擬路徑與 GUID。</summary>
	public static bool Register(NVirtualPath path, GUID guid)
	{
		if (path.IsEmpty || guid.IsZero)
		{
			Console.WriteLine($"[AssetDatabase] Register: path empty or guid zero");
			return false;
		}

		Console.WriteLine($"[AssetDatabase] Register: path={path.FullPath}, guid={guid}");

		if (TryRegisterNative(path, guid))
		{
			Cache(path, guid);
			Console.WriteLine($"[AssetDatabase] Register: native success");
			return true;
		}

	#if DEBUG
		if (s_pathToGuid.ContainsKey(path))
		{
			return false;
		}

		Cache(path, guid);
		return true;
	#else
		return false;
	#endif
	}

	/// <summary>依路徑解析 GUID。</summary>
	public static bool TryResolveGuid(NVirtualPath path, out GUID guid)
	{
		guid = GUID.Zero;
		if (path.IsEmpty)
		{
			return false;
		}

		if (TryResolveGuidNative(path, out guid))
		{
			return true;
		}

	#if DEBUG
		return s_pathToGuid.TryGetValue(path, out guid);
	#else
		return false;
	#endif
	}

	/// <summary>依 GUID 解析虛擬路徑。</summary>
	public static bool TryResolvePath(GUID guid, out NVirtualPath path)
	{
		path = default;
		if (guid.IsZero)
		{
			return false;
		}

		string? nativePath;
		if (TryResolvePathNative(guid, out nativePath))
		{
			path = new NVirtualPath(nativePath!);
			return true;
		}

	#if DEBUG
		return s_guidToPath.TryGetValue(guid.ToHexString(), out path);
	#else
		return false;
	#endif
	}

	private static void Cache(NVirtualPath path, GUID guid)
	{
		s_pathToGuid[path] = guid;
		s_guidToPath[guid.ToHexString()] = path;
	}

	private static unsafe bool TryRegisterNative(NVirtualPath path, GUID guid)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			Console.WriteLine($"[AssetDatabase] TryRegisterNative: IsInstalled=false");
			return false;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.RegisterAsset;
		if (fn == null)
		{
			Console.WriteLine($"[AssetDatabase] TryRegisterNative: RegisterAsset fn=null");
			return false;
		}

		var native = guid.ToNative();
		var bytes = Encoding.UTF8.GetBytes(path.FullPath);
		fixed (byte* p = bytes)
		{
			var result = fn(p, native);
			if (result != 0)
			{
				Console.WriteLine($"[AssetDatabase] TryRegisterNative FAILED: path={path.FullPath}, guid={guid}, result={result}");
			}
			return result == 0;
		}
	}

	private static unsafe bool TryResolveGuidNative(NVirtualPath path, out GUID guid)
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

		var bytes = Encoding.UTF8.GetBytes(path.FullPath);
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
