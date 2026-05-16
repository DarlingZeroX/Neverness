namespace VisionGal.Managed.Entity;

/// <summary>
/// 可讀顯示名稱：用於除錯、UI 標籤或劇本顯示；<b>非</b>資產 GUID、亦非跨場景唯一 Id。
/// </summary>
public sealed class NameComponent : VGComponent
{
	/// <inheritdoc />
	public NameComponent(EntityHandle entity)
		: base(entity)
	{
		DisplayName = string.Empty;
	}

	/// <summary>人類可讀字串；允許空字串表示「尚未命名」。</summary>
	public string DisplayName { get; set; } = string.Empty;
}
