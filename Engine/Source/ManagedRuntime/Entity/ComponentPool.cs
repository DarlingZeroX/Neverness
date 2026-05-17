namespace Neverness.Managed.Entity;

/// <summary>
/// 泛型元件（或中介緩衝物件）之輕量堆疊池：預留給高頻 Rent/Return 場景；首包不與 <see cref="EntityWorld"/> 自動整合。
/// </summary>
/// <remarks>
/// <para>多數 <see cref="VGComponent"/> 子類別需在建構子綁定 <see cref="EntityHandle"/>，與「無狀態物件池」語意僅部分重疊；
/// 呼叫端若池化元件，歸還前務必清除欄位並確保不再被任何 <see cref="EntityWorld"/> 引用。</para>
/// <para>未來若遷移至 SoA 或 Archetype 批次配置，可能由此類或專用分配器承接緩衝區重用。</para>
/// </remarks>
/// <typeparam name="T">參考型別池化成員；實值型別請另評估是否裝箱。</typeparam>
public sealed class ComponentPool<T>
	where T : class
{
	private readonly Stack<T> _free = new();

	/// <summary>若池非空則彈出頂端實例；否則回傳 null（呼叫端可自行 new 或改用帶工廠之重載）。</summary>
	public T? TryRent() => _free.Count > 0 ? _free.Pop() : null;

	/// <summary>
	/// 將實例推回池中；不檢查重複歸還或與實體世界之關聯，呼叫端須自行保證生命週期安全。
	/// </summary>
	public void Return(T instance)
	{
		ArgumentNullException.ThrowIfNull(instance);
		_free.Push(instance);
	}

	/// <summary>釋放池內所有引用，協助 GC；測試隔離時呼叫。</summary>
	public void Clear() => _free.Clear();
}
