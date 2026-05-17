using VisionGal.Managed.Reflection;

namespace VisionGal.Managed.Inspector;

/// <summary>
/// Inspector 視圖模型：綁定目標物件與其 <see cref="TypeMetadata"/> 可編輯屬性清單。
/// </summary>
public sealed class InspectorView
{
	/// <summary>目前檢視中的目標實例。</summary>
	public object? Target { get; private set; }

	/// <summary>目標型別元資料。</summary>
	public TypeMetadata? Metadata { get; private set; }

	/// <summary>應在 Inspector 顯示的屬性（排除 HideInInspector）。</summary>
	public IReadOnlyList<PropertyMetadata> VisibleProperties { get; private set; } = [];

	/// <summary>綁定新目標並刷新可見屬性。</summary>
	public void Bind(object? target)
	{
		Target = target;
		if (target == null)
		{
			Metadata = null;
			VisibleProperties = [];
			return;
		}

		Metadata = ReflectionRegistry.GetOrCreate(target.GetType());
		VisibleProperties = Metadata.SerializableProperties
			.Where(p => !p.HideInInspector)
			.ToList();
	}
}
