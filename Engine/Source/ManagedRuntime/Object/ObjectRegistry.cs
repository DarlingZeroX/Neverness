namespace VisionGal.Managed.Object;

/// <summary>
/// 進程內託管 <see cref="VGObject"/> 實例之靜態註冊表（<see cref="VGObjectId"/> → 物件）。
/// 執行緒：Bootstrap 與主遊戲執行緒單執行緒存取；未來可外掛讀寫鎖。
/// </summary>
public static class ObjectRegistry
{
	private static readonly Dictionary<VGObjectId, VGObject> s_objects = new();
	private static ulong s_nextId = 1;

	/// <summary>目前註冊之物件數量。</summary>
	public static int Count => s_objects.Count;

	/// <summary>分配下一個唯一 <see cref="VGObjectId"/>。</summary>
	public static VGObjectId AllocateId() => new(s_nextId++);

	/// <summary>將物件註冊至表內；若識別碼已存在則拋出例外。</summary>
	public static void Register(VGObject obj)
	{
		ArgumentNullException.ThrowIfNull(obj);
		if (!obj.Id.IsValid)
		{
			throw new ArgumentException("物件識別碼無效。", nameof(obj));
		}

		if (!s_objects.TryAdd(obj.Id, obj))
		{
			throw new InvalidOperationException($"識別碼 {obj.Id} 已註冊。");
		}
	}

	/// <summary>依識別碼移除註冊；不存在時靜默返回。</summary>
	public static void Unregister(VGObjectId id)
	{
		s_objects.Remove(id);
	}

	/// <summary>嘗試取得已註冊物件。</summary>
	public static bool TryGet(VGObjectId id, out VGObject? obj) => s_objects.TryGetValue(id, out obj);

	/// <summary>取得已註冊物件；不存在時拋出 <see cref="KeyNotFoundException"/>。</summary>
	public static VGObject GetRequired(VGObjectId id) =>
		s_objects.TryGetValue(id, out var obj) ? obj : throw new KeyNotFoundException($"未找到 VGObjectId={id}");

	/// <summary>
	/// 清空註冊表（Bootstrap 與測試重置）：先對所有已註冊物件呼叫 <see cref="VGObject.Dispose"/> 以釋放 Native 控制代碼，再重置 Id 計數器。
	/// </summary>
	public static void ClearForTesting()
	{
		foreach (var obj in s_objects.Values.ToArray())
		{
			obj.Dispose();
		}

		s_objects.Clear();
		s_nextId = 1;
	}
}
