namespace Neverness.Editor.Framework.Reflection;

/// <summary>
/// 标记字段或属性应参与序列化（即使为 private）；对齐 Unity <c>[SerializeField]</c> 语义（仅 Editor/工具链）。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SerializeFieldAttribute : Attribute
{
}

/// <summary>
/// 标记成员不应在 Inspector 中显示。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class HideInInspectorAttribute : Attribute
{
}

/// <summary>
/// 为数值属性指定 Inspector 滑杆范围。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class RangeAttribute : Attribute
{
	/// <summary>范围下限。</summary>
	public float Min { get; }

	/// <summary>范围上限。</summary>
	public float Max { get; }

	/// <summary>建立范围约束。</summary>
	public RangeAttribute(float min, float max)
	{
		Min = min;
		Max = max;
	}
}
