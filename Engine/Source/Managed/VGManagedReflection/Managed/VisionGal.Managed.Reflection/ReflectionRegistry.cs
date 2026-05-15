namespace VisionGal.Managed.Reflection;

/// <summary>
/// 快取 <see cref="Type"/> → <see cref="TypeMetadata"/>，避免重複反射掃描。
/// </summary>
public static class ReflectionRegistry
{
	private static readonly Dictionary<Type, TypeMetadata> s_cache = new();

	/// <summary>取得或建立指定型別之元資料。</summary>
	public static TypeMetadata GetOrCreate(Type type)
	{
		ArgumentNullException.ThrowIfNull(type);
		if (!s_cache.TryGetValue(type, out var meta))
		{
			meta = new TypeMetadata(type);
			s_cache[type] = meta;
		}

		return meta;
	}

	/// <summary>泛型便利方法。</summary>
	public static TypeMetadata GetOrCreate<T>() => GetOrCreate(typeof(T));

	/// <summary>清空快取（測試用）。</summary>
	internal static void ClearForTesting() => s_cache.Clear();
}
