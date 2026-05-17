using Neverness.Managed.Engine;

namespace Neverness.Managed.Assets;

/// <summary>
/// 資產依賴關係追蹤；經 Native AssetRegistry 或託管側快取查詢。
/// </summary>
public static unsafe class DependencyTracking
{
	private static readonly Dictionary<string, List<GUID>> s_dependencies = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>記錄資產依賴（託管快取）。</summary>
	public static void RecordDependency(GUID asset, GUID dependency)
	{
		if (asset.IsZero || dependency.IsZero)
		{
			return;
		}

		var key = asset.ToHexString();
		if (!s_dependencies.TryGetValue(key, out var list))
		{
			list = [];
			s_dependencies[key] = list;
		}

		if (!list.Contains(dependency))
		{
			list.Add(dependency);
		}
	}

	/// <summary>取得依賴數量（優先 Native）。</summary>
	public static uint GetDependencyCount(GUID asset)
	{
		if (asset.IsZero)
		{
			return 0;
		}

		if (EngineNativeApiBootstrap.IsInstalled)
		{
			var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.GetDependencyCount;
			if (fn != null)
			{
				return fn(asset.ToNative());
			}
		}

		return (uint)(s_dependencies.TryGetValue(asset.ToHexString(), out var list) ? list.Count : 0);
	}

	/// <summary>取得指定索引之依賴 GUID。</summary>
	public static bool TryGetDependencyAt(GUID asset, uint index, out GUID dependency)
	{
		dependency = GUID.Zero;
		if (asset.IsZero)
		{
			return false;
		}

		if (EngineNativeApiBootstrap.IsInstalled)
		{
			var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.GetDependencyAt;
			if (fn != null)
			{
				unsafe
				{
					VGGuid native;
					if (fn(asset.ToNative(), index, &native) == 0)
					{
						dependency = GUID.FromNative(native);
						return !dependency.IsZero;
					}
				}
			}
		}

		if (s_dependencies.TryGetValue(asset.ToHexString(), out var list) && index < list.Count)
		{
			dependency = list[(int)index];
			return true;
		}

		return false;
	}

	internal static void ClearForTesting() => s_dependencies.Clear();
}
