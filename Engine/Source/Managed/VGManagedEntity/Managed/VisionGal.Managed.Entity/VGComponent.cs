namespace VisionGal.Managed.Entity;

/// <summary>
/// 託管元件抽象基類：掛載於單一 <see cref="EntityHandle"/> 上，由 <see cref="EntityWorld"/> 以字典持有其生命週期（與實體同存亡，除非個別 <see cref="EntityWorld.RemoveComponent{T}"/>）。
/// </summary>
/// <remarks>
/// <para>對齊路線圖「Managed Component System」之第一步；首包不帶序列化、反射管線或系統排程。</para>
/// <para>未來可拆出獨立程式集 <c>VisionGal.Managed.Component</c>，並與 <see cref="ComponentRegistry"/> 工廠、編輯器資料驅動載入對齊。</para>
/// </remarks>
public abstract class VGComponent
{
	/// <summary>
	/// 建立並記錄所屬實體代碼；子類別應將 <paramref name="entity"/> 原樣傳入 <c>base(entity)</c>。
	/// </summary>
	/// <param name="entity">建議為剛 <see cref="EntityWorld.Spawn"/> 且仍存活之代碼；若日後重用 Index，仍須與當前世代一致。</param>
	protected VGComponent(EntityHandle entity)
	{
		Entity = entity;
	}

	/// <summary>
	/// 建構時綁定之實體代碼（值型拷貝語意）。注意：實體可能稍後被 <see cref="EntityWorld.Destroy"/>，
	/// 此屬性不會自動更新；呼叫端應以 <see cref="EntityWorld.IsAlive"/> 判斷是否仍可信賴。
	/// </summary>
	public EntityHandle Entity { get; }
}
