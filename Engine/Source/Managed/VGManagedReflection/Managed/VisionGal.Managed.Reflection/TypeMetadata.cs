using System.Reflection;

namespace VisionGal.Managed.Reflection;

/// <summary>
/// 單一 CLR 型別之可序列化成員集合與型別級屬性摘要。
/// </summary>
public sealed class TypeMetadata
{
	/// <summary>目標 CLR 型別。</summary>
	public Type ClrType { get; }

	/// <summary>可序列化屬性清單（含 public 欄位與標記 SerializeField 之 private 欄位）。</summary>
	public IReadOnlyList<PropertyMetadata> SerializableProperties { get; }

	/// <summary>建立型別元資料並掃描成員。</summary>
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
			// 僅 [SerializeField] 或具 public setter 之可讀寫屬性納入序列化掃描。
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
