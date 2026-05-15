namespace VisionGal.Managed.Gameplay;

/// <summary>
/// Galgame 執行時變數表（託管字典）；Phase 6 首包，尚未與存檔或 Native ABI 對接。
/// </summary>
public sealed class GameplayVariableStore
{
	private readonly Dictionary<string, object?> _values = new(StringComparer.Ordinal);

	/// <summary>目前變數數量。</summary>
	public int Count => _values.Count;

	/// <summary>設定變數值；<paramref name="name"/> 不可為空白。</summary>
	public void Set(string name, object? value)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		_values[name] = value;
	}

	/// <summary>取得變數值；不存在時拋出 <see cref="KeyNotFoundException"/>。</summary>
	public object? Get(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		return _values[name];
	}

	/// <summary>嘗試取得變數值。</summary>
	public bool TryGet(string name, out object? value)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		return _values.TryGetValue(name, out value);
	}

	/// <summary>移除變數；不存在時靜默返回。</summary>
	public void Remove(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		_values.Remove(name);
	}

	/// <summary>清空所有變數（測試或場景切換用）。</summary>
	public void Clear() => _values.Clear();
}
