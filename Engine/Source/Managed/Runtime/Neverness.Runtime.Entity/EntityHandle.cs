namespace Neverness.Managed.Entity;

/// <summary>
/// 實體控制代碼（Handle）：以單調遞增之 <see cref="Index"/> 標識槽位，並以 <see cref="Generation"/> 記錄該槽位「第幾代」存活週期。
/// </summary>
/// <remarks>
/// <para><b>為何需要 Generation</b>：銷毀實體後，同一 <see cref="Index"/> 可能被 <see cref="EntityWorld.Spawn"/> 再次分配給新實體。
/// 若呼叫端仍持有舊代碼，僅比對 Index 會誤判為同一實體；比對 <see cref="Generation"/> 則可偵測懸掛引用並拒絕操作。</para>
/// <para>P0 首包為純託管實作，尚無 Native <b>NNEntityHandle</b> 對齊；未來若與 C++ 並表，應在本型別或專用轉換層完成 ABI 映射與版本欄位約定。</para>
/// </remarks>
public readonly struct EntityHandle : IEquatable<EntityHandle>
{
	/// <summary>
	/// 全域約定之「無效」代碼：<see cref="Index"/> 為 0、<see cref="Generation"/> 為 0。
	/// 不得用於 <see cref="EntityWorld"/> 查表；亦常用於 <see cref="HierarchyComponent.Parent"/> 表示「無父節點」。
	/// </summary>
	public static EntityHandle Invalid => default;

	/// <summary>
	/// 實體槽位索引：由 <see cref="EntityWorld"/> 自 1 起單調遞增分配；值 0 保留給 <see cref="Invalid"/>。
	/// </summary>
	public uint Index { get; init; }

	/// <summary>
	/// 世代計數：同一 Index 每經歷一次完整 <see cref="EntityWorld.Destroy"/> 即遞增，使舊 Handle 與新 Spawn 區分開。
	/// </summary>
	public uint Generation { get; init; }

	/// <summary>當 <see cref="Index"/> 非 0 時視為語意上「可參與查表」之代碼；實際是否仍存活須以 <see cref="EntityWorld.IsAlive"/> 為準。</summary>
	public bool IsValid => Index != 0;

	/// <inheritdoc />
	public bool Equals(EntityHandle other) => Index == other.Index && Generation == other.Generation;

	/// <inheritdoc />
	public override bool Equals(object? obj) => obj is EntityHandle other && Equals(other);

	/// <inheritdoc />
	public override int GetHashCode() => HashCode.Combine(Index, Generation);

	/// <summary>相等運算子。</summary>
	public static bool operator ==(EntityHandle left, EntityHandle right) => left.Equals(right);

	/// <summary>不等運算子。</summary>
	public static bool operator !=(EntityHandle left, EntityHandle right) => !left.Equals(right);
}
