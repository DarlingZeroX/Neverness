using System.Reflection;

namespace Neverness.Editor.Framework.Reflection;

/// <summary>
/// 单一可序列化属性之反射元数据（名称、类型、属性标记）。
/// </summary>
public sealed class PropertyMetadata
{
	/// <summary>成员名称。</summary>
	public string Name { get; }

	/// <summary>成员 CLR 类型。</summary>
	public Type PropertyType { get; }

	/// <summary>是否标记 <see cref="SerializeFieldAttribute"/>。</summary>
	public bool SerializeField { get; }

	/// <summary>是否标记 <see cref="HideInInspectorAttribute"/>。</summary>
	public bool HideInInspector { get; }

	/// <summary>可选之 <see cref="RangeAttribute"/> 范围。</summary>
	public RangeAttribute? Range { get; }

	/// <summary>底层 <see cref="MemberInfo"/>（字段或属性）。</summary>
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

	/// <summary>从实例读取成员值。</summary>
	public object? GetValue(object target) =>
		Member switch
		{
			FieldInfo f => f.GetValue(target),
			PropertyInfo p when p.CanRead => p.GetValue(target),
			_ => null
		};

	/// <summary>向实例写入成员值。</summary>
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
