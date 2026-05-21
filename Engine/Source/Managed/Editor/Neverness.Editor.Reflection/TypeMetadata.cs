using System.Reflection;

namespace Neverness.Editor.Framework.Reflection;

/// <summary>
/// 单一 CLR 类型之可序列化成员集合与类型级属性摘要（Editor 专用）。
/// </summary>
public sealed class TypeMetadata
{
	/// <summary>目标 CLR 类型。</summary>
	public Type ClrType { get; }

	/// <summary>可序列化属性列表。</summary>
	public IReadOnlyList<PropertyMetadata> SerializableProperties { get; }

	/// <summary>建立类型元数据并扫描成员。</summary>
	public TypeMetadata(Type clrType)
	{
		ClrType = clrType;
		SerializableProperties = ScanMembers(clrType);
	}

	private static List<PropertyMetadata> ScanMembers(Type type)
	{
		var list = new List<PropertyMetadata>();
		const BindingFlags flags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;

		foreach (var field in type.GetFields(flags))
		{
			var serialize = field.IsPublic || field.IsDefined(typeof(SerializeFieldAttribute), inherit: false);
			if (!serialize)
			{
				continue;
			}

			list.Add(new PropertyMetadata(
				field,
				field.FieldType,
				field.IsDefined(typeof(SerializeFieldAttribute), inherit: false),
				field.IsDefined(typeof(HideInInspectorAttribute), inherit: false),
				field.GetCustomAttribute<RangeAttribute>()));
		}

		foreach (var prop in type.GetProperties(flags))
		{
			if (!prop.CanRead || !prop.CanWrite)
			{
				continue;
			}

			var hasSerializeField = prop.IsDefined(typeof(SerializeFieldAttribute), inherit: false);
			var setter = prop.GetSetMethod(nonPublic: true);
			var hasPublicSetter = setter is { IsPublic: true };
			if (!hasSerializeField && !hasPublicSetter)
			{
				continue;
			}

			list.Add(new PropertyMetadata(
				prop,
				prop.PropertyType,
				hasSerializeField,
				prop.IsDefined(typeof(HideInInspectorAttribute), inherit: false),
				prop.GetCustomAttribute<RangeAttribute>()));
		}

		return list;
	}
}
