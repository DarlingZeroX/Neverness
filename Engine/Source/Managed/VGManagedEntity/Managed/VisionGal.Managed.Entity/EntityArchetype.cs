namespace VisionGal.Managed.Entity;

/// <summary>
/// 實體原型（Archetype）枚舉占位：描述一類實體預期掛載之元件組合語意，供文件化與未來批次分配器對齊。
/// </summary>
/// <remarks>
/// 首包不強制 <see cref="EntityWorld.Spawn"/> 依此枚舉自動掛件；呼叫端可手動遵守約定或於後續版本接入專用 <c>Spawn(archetype)</c> API。
/// </remarks>
public enum EntityArchetype
{
	/// <summary>無預設元件組合，由呼叫端自行 <see cref="EntityWorld.AddComponent{T}"/>。</summary>
	Empty = 0,

	/// <summary>
	/// 常見「具空間座標且可命名」實體之語意標籤：預期搭配 <see cref="TransformComponent"/> 與 <see cref="NameComponent"/>，實際掛載仍須顯式呼叫。
	/// </summary>
	SpatialNamed = 1,
}
