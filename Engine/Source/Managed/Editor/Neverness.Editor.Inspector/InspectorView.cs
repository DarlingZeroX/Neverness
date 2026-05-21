using Neverness.Editor.Framework.Reflection;

namespace Neverness.Editor.Framework.Inspector;

/// <summary>
/// Inspector 视图模型：绑定目标对象与其 <see cref="TypeMetadata"/> 可编辑属性列表。
/// </summary>
public sealed class InspectorView
{
	/// <summary>目前检视中的目标实例。</summary>
	public object? Target { get; private set; }

	/// <summary>目标类型元数据。</summary>
	public TypeMetadata? Metadata { get; private set; }

	/// <summary>应在 Inspector 显示的属性（排除 HideInInspector）。</summary>
	public IReadOnlyList<PropertyMetadata> VisibleProperties { get; private set; } = [];

	/// <summary>绑定新目标并刷新可见属性。</summary>
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
