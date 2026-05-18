using System.Reflection;

namespace Neverness.Managed.Reflection;

/// <summary>
/// 單一可序列化屬性之反射元資料（名稱、型別、屬性標記）。
/// </summary>
public sealed class PropertyMetadata
{
	/// <summary>成員名稱。</summary>
	public string Name { get; }

	/// <summary>成員 CLR 型別。</summary>
	public Type PropertyType { get; }

	/// <summary>是否標記 <see cref="SerializeFieldAttribute"/>。</summary>
	public bool SerializeField { get; }

	/// <summary>是否標記 <see cref="HideInInspectorAttribute"/>。</summary>
	public bool HideInInspector { get; }

	/// <summary>可選之 <see cref="RangeAttribute"/> 範圍。</summary>
	public RangeAttribute? Range { get; }

	/// <summary>底層 <see cref="MemberInfo"/>（欄位或屬性）。</summary>
	public MemberInfo Member { get; }

	internal PropertyMetadata(MemberInfo member, Type propertyType, bool serializeField, bool hideInInspector, RangeAttribute? range)
	{
		Member = member;
		Name = member.Name;
		PropertyType = propertyType;
		SerializeField = serializeField;
		HideInInspector = hideInInspector;
		Range = range;
	}

	/// <summary>自實例讀取成員值。</summary>
	public object? GetValue(object target)
	{
		return Member switch
		{
			FieldInfo f => f.GetValue(target),
			PropertyInfo p when p.CanRead => p.GetValue(target),
			_ => null
		};
	}

	/// <summary>向實例寫入成員值。</summary>
	public void SetValue(object target, object? value)
	{
		switch (Member)
		{
			case FieldInfo f:
				f.SetValue(target, value);
				break;
			case PropertyInfo p when p.CanWrite:
				p.SetValue(target, value);
				break;
		}
	}
}
