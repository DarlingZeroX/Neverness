namespace VisionGal.Managed.Reflection;

/// <summary>
/// 標記欄位或屬性應參與序列化（即使為 private）；對齊 Unity <c>[SerializeField]</c> 語義。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SerializeFieldAttribute : Attribute
{
}

/// <summary>
/// 標記成員不應在 Inspector 中顯示。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class HideInInspectorAttribute : Attribute
{
}

/// <summary>
/// 為數值屬性指定 Inspector 滑桿範圍。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class RangeAttribute : Attribute
{
	/// <summary>範圍下限。</summary>
	public float Min { get; }

	/// <summary>範圍上限。</summary>
	public float Max { get; }

	/// <summary>建立範圍約束。</summary>
	public RangeAttribute(float min, float max)
	{
		Min = min;
		Max = max;
	}
}
