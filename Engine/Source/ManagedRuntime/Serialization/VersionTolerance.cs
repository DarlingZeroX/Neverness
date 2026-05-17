using System.Text.Json;
using System.Text.Json.Serialization;

namespace VisionGal.Managed.Serialization;

/// <summary>
/// 序列化版本容忍策略：讀取時忽略未知欄位、寫入時附帶格式版本號。
/// </summary>
public static class VersionTolerance
{
	/// <summary>當前序列化格式版本（遞增於破壞性變更）。</summary>
	public const int CurrentFormatVersion = 1;

	/// <summary>共用 <see cref="JsonSerializerOptions"/>：寬鬆讀取、駝峰命名、枚舉字串化。</summary>
	public static JsonSerializerOptions CreateOptions() => new()
	{
		PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
		WriteIndented = true,
		DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
		PropertyNameCaseInsensitive = true,
		ReadCommentHandling = JsonCommentHandling.Skip,
		AllowTrailingCommas = true,
		Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
	};
}
