using System.Text.Json;
using System.Text.Json.Serialization;

namespace Neverness.Editor.Framework.Serialization;

/// <summary>
/// Editor 序列化版本容忍策略：读取时忽略未知字段、写入时附带格式版本号。
/// </summary>
public static class VersionTolerance
{
	/// <summary>当前序列化格式版本（递增于破坏性变更）。</summary>
	public const int CurrentFormatVersion = 1;

	/// <summary>共用 <see cref="JsonSerializerOptions"/>。</summary>
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
