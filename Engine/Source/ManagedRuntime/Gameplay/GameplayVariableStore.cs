namespace VisionGal.Managed.Gameplay;

/// <summary>
/// Galgame 執行時變數表：託管端鍵值字典，供劇本、<see cref="SequenceRunner"/> 與存檔快照共用。
/// </summary>
/// <remarks>
/// Phase 6：記憶體 API 於本檔；JSON 往返（<c>formatVersion</c>、型別標籤）見同名 partial <c>GameplayVariableStore.Serialization.cs</c>。
/// </remarks>
public sealed partial class GameplayVariableStore
{
	private readonly Dictionary<string, object?> _values = new(StringComparer.Ordinal);

	/// <summary>當前已儲存的變數條目數量。</summary>
	public int Count => _values.Count;

	/// <summary>
	/// 寫入或覆寫變數；鍵名不可為 null/空白。
	/// </summary>
	/// <remarks>持久化請使用 <c>ToJson</c> / <c>TryParseFromJson</c>（與場景等 DTO 一樣走版本容忍選項）。</remarks>
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

	/// <summary>
	/// 以另一張變數表覆寫本表：先清空再複製所有鍵值（供 <see cref="GameplaySessionSnapshot.ApplyTo"/> 等載入路徑使用）。
	/// </summary>
	/// <param name="source">來源表，不可為 null。</param>
	public void CopyFrom(GameplayVariableStore source)
	{
		ArgumentNullException.ThrowIfNull(source);
		_values.Clear();
		foreach (var kv in source._values)
		{
			_values[kv.Key] = kv.Value;
		}
	}
}
