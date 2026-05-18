using System.Text.Json;
using Neverness.Managed.Reflection;

namespace Neverness.Managed.Serialization;

/// <summary>
/// 場景描述資料之 JSON 序列化/反序列化（基於 <see cref="System.Text.Json"/>）。
/// </summary>
public static class SceneSerializer
{
	/// <summary>場景文件根 DTO。</summary>
	public sealed class SceneDocument
	{
		/// <summary>格式版本。</summary>
		public int FormatVersion { get; set; } = VersionTolerance.CurrentFormatVersion;

		/// <summary>場景名稱。</summary>
		public string Name { get; set; } = string.Empty;

		/// <summary>實體條目（型別名 + 屬性字典）。</summary>
		public List<SceneEntityEntry> Entities { get; set; } = [];
	}

	/// <summary>單一實體序列化條目。</summary>
	public sealed class SceneEntityEntry
	{
		/// <summary>實體名稱。</summary>
		public string Name { get; set; } = string.Empty;

		/// <summary>CLR 型別全名。</summary>
		public string TypeName { get; set; } = string.Empty;

		/// <summary>可序列化屬性鍵值。</summary>
		public Dictionary<string, JsonElement> Properties { get; set; } = new();
	}

	/// <summary>將場景文件序列化為 UTF-8 JSON 字串。</summary>
	public static string Serialize(SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		document.FormatVersion = VersionTolerance.CurrentFormatVersion;
		return JsonSerializer.Serialize(document, VersionTolerance.CreateOptions());
	}

	/// <summary>自 JSON 字串還原場景文件。</summary>
	public static SceneDocument? Deserialize(string json)
	{
		if (string.IsNullOrWhiteSpace(json))
		{
			return null;
		}

		return JsonSerializer.Deserialize<SceneDocument>(json, VersionTolerance.CreateOptions());
	}

	/// <summary>依 <see cref="ReflectionRegistry"/> 將任意物件之可序列化欄位寫入條目。</summary>
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

	/// <summary>
	/// 將 <see cref="SceneEntityEntry.Properties"/> 內之 JSON 值寫回已存在之託管實例（依 <see cref="ReflectionRegistry"/> 可序列化成員清單）。
	/// </summary>
	/// <param name="instance">目標物件（通常為 <see cref="Neverness.Managed.Object.VGObject"/> 衍生）。</param>
	/// <param name="entry">含屬性字典之實體條目。</param>
	/// <remarks>
	/// 僅處理 metadata 中已登記之屬性名；條目中多餘或未知鍵會略過；缺少之可選鍵不拋出例外。
	/// </remarks>
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
