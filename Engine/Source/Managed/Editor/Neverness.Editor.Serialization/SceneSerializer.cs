using System.Text.Json;
using Neverness.Editor.Framework.Reflection;

namespace Neverness.Editor.Framework.Serialization;

/// <summary>
/// 场景描述数据之 JSON 序列化/反序列化（Editor/工具链；基于 <see cref="System.Text.Json"/>）。
/// </summary>
public static class SceneSerializer
{
	/// <summary>场景文件根 DTO。</summary>
	public sealed class SceneDocument
	{
		/// <summary>格式版本。</summary>
		public int FormatVersion { get; set; } = VersionTolerance.CurrentFormatVersion;

		/// <summary>场景名称。</summary>
		public string Name { get; set; } = string.Empty;

		/// <summary>实体条目（类型名 + 属性字典）。</summary>
		public List<SceneEntityEntry> Entities { get; set; } = [];
	}

	/// <summary>单一实体序列化条目。</summary>
	public sealed class SceneEntityEntry
	{
		/// <summary>实体名称。</summary>
		public string Name { get; set; } = string.Empty;

		/// <summary>CLR 类型全名。</summary>
		public string TypeName { get; set; } = string.Empty;

		/// <summary>可序列化属性键值。</summary>
		public Dictionary<string, JsonElement> Properties { get; set; } = new();
	}

	/// <summary>将场景文件序列化为 UTF-8 JSON 字符串。</summary>
	public static string Serialize(SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		document.FormatVersion = VersionTolerance.CurrentFormatVersion;
		return JsonSerializer.Serialize(document, VersionTolerance.CreateOptions());
	}

	/// <summary>自 JSON 字符串还原场景文件。</summary>
	public static SceneDocument? Deserialize(string json)
	{
		if (string.IsNullOrWhiteSpace(json))
		{
			return null;
		}

		return JsonSerializer.Deserialize<SceneDocument>(json, VersionTolerance.CreateOptions());
	}

	/// <summary>依 <see cref="ReflectionRegistry"/> 将任意对象的可序列化字段写入条目。</summary>
	public static SceneEntityEntry CaptureObject(string name, object instance)
	{
		ArgumentNullException.ThrowIfNull(instance);
		var meta = ReflectionRegistry.GetOrCreate(instance.GetType());
		var entry = new SceneEntityEntry
		{
			Name = name,
			TypeName = instance.GetType().AssemblyQualifiedName ?? instance.GetType().FullName ?? instance.GetType().Name
		};

		foreach (var prop in meta.SerializableProperties)
		{
			var value = prop.GetValue(instance);
			entry.Properties[prop.Name] = JsonSerializer.SerializeToElement(value, VersionTolerance.CreateOptions());
		}

		return entry;
	}

	/// <summary>将条目中的 JSON 值写回已存在的托管实例。</summary>
	public static void ApplyEntryProperties(object instance, SceneEntityEntry entry)
	{
		ArgumentNullException.ThrowIfNull(instance);
		ArgumentNullException.ThrowIfNull(entry);

		var meta = ReflectionRegistry.GetOrCreate(instance.GetType());
		var options = VersionTolerance.CreateOptions();

		foreach (var prop in meta.SerializableProperties)
		{
			if (!entry.Properties.TryGetValue(prop.Name, out var element))
			{
				continue;
			}

			object? value;
			if (element.ValueKind == JsonValueKind.Null)
			{
				value = null;
			}
			else
			{
				value = JsonSerializer.Deserialize(element.GetRawText(), prop.PropertyType, options);
			}

			prop.SetValue(instance, value);
		}
	}
}
