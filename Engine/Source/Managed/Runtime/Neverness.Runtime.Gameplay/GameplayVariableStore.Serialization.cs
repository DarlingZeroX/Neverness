using System.Diagnostics.CodeAnalysis;
using System.Text.Json;
using System.Text.Json.Serialization;
using Neverness.Managed.Serialization;

namespace Neverness.Managed.Gameplay;

/// <summary>
/// <see cref="GameplayVariableStore"/> 之 JSON 快照（MVP）：與 <see cref="VersionTolerance"/> 共用選項，寫入 <c>formatVersion</c>，讀取時忽略未知根欄位（由選項控制）。
/// </summary>
/// <remarks>
/// 與場景/資產 DTO 一致採寬鬆反序列化策略，便於日後欄位擴充；不支援之變數值型別於 <see cref="GameplayVariableStore.ToJson"/> 拋出明確例外。
/// </remarks>
public sealed partial class GameplayVariableStore
{
	private const string KindString = "string";
	private const string KindBool = "bool";
	private const string KindInt64 = "int64";
	private const string KindDouble = "double";

	/// <summary>
	/// 將當前變數表序列化為 JSON 字串（含 <c>formatVersion</c> 與 <c>entries</c>）。
	/// </summary>
	/// <remarks>
	/// 僅支援值型別：<see cref="string"/>、<see cref="bool"/>、整數（寫為 int64）、<see cref="double"/>/<see cref="float"/>。
	/// 其他型別將觸發 <see cref="InvalidOperationException"/>，避免靜默丟失資料。
	/// </remarks>
	public string ToJson()
	{
		var dto = new GameplayVariablesDocumentDto
		{
			FormatVersion = VersionTolerance.CurrentFormatVersion,
			Entries = new List<GameplayVariableEntryDto>(_values.Count),
		};

		foreach (var kv in _values)
		{
			dto.Entries.Add(CreateEntryDto(kv.Key, kv.Value));
		}

		return JsonSerializer.Serialize(dto, VersionTolerance.CreateOptions());
	}

	/// <summary>
	/// 自 JSON 還原新的變數表；成功時 <paramref name="store"/> 非 null。
	/// </summary>
	/// <param name="json">UTF-16 JSON 文字。</param>
	/// <param name="store">輸出之新實例。</param>
	/// <returns>格式合法且版本可接受時為 true。</returns>
	/// <remarks>
	/// 接受 <c>formatVersion</c> 為 1..<see cref="VersionTolerance.CurrentFormatVersion"/>；
	/// 條目型別僅處理 <c>string|bool|int64|double</c>，未知 <c>kind</c> 則整份解析失敗。
	/// </remarks>
	public static bool TryParseFromJson(ReadOnlySpan<char> json, [NotNullWhen(true)] out GameplayVariableStore? store)
	{
		store = null;
		GameplayVariablesDocumentDto? doc;
		try
		{
			doc = JsonSerializer.Deserialize<GameplayVariablesDocumentDto>(json, VersionTolerance.CreateOptions());
		}
		catch (JsonException)
		{
			return false;
		}

		if (doc?.Entries is null)
		{
			return false;
		}

		if (doc.FormatVersion < 1 || doc.FormatVersion > VersionTolerance.CurrentFormatVersion)
		{
			return false;
		}

		var result = new GameplayVariableStore();
		foreach (var e in doc.Entries)
		{
			if (string.IsNullOrWhiteSpace(e.Name) || string.IsNullOrWhiteSpace(e.Kind))
			{
				return false;
			}

			switch (e.Kind)
			{
				case KindString:
					if (e.StringValue is null)
					{
						return false;
					}

					result.Set(e.Name, e.StringValue);
					break;
				case KindBool:
					if (!e.BoolValue.HasValue)
					{
						return false;
					}

					result.Set(e.Name, e.BoolValue.Value);
					break;
				case KindInt64:
					if (!e.Int64Value.HasValue)
					{
						return false;
					}

					result.Set(e.Name, e.Int64Value.Value);
					break;
				case KindDouble:
					if (!e.DoubleValue.HasValue)
					{
						return false;
					}

					result.Set(e.Name, e.DoubleValue.Value);
					break;
				default:
					return false;
			}
		}

		store = result;
		return true;
	}

	private static GameplayVariableEntryDto CreateEntryDto(string name, object? value)
	{
		var entry = new GameplayVariableEntryDto { Name = name };
		switch (value)
		{
			case string s:
				entry.Kind = KindString;
				entry.StringValue = s;
				break;
			case bool b:
				entry.Kind = KindBool;
				entry.BoolValue = b;
				break;
			case long l:
				entry.Kind = KindInt64;
				entry.Int64Value = l;
				break;
			case int i:
				entry.Kind = KindInt64;
				entry.Int64Value = i;
				break;
			case short sh:
				entry.Kind = KindInt64;
				entry.Int64Value = sh;
				break;
			case byte by:
				entry.Kind = KindInt64;
				entry.Int64Value = by;
				break;
			case double d:
				entry.Kind = KindDouble;
				entry.DoubleValue = d;
				break;
			case float f:
				entry.Kind = KindDouble;
				entry.DoubleValue = f;
				break;
			default:
				throw new InvalidOperationException(
					$"GameplayVariableStore.ToJson：不支援的變數型別 '{value?.GetType().FullName ?? "null"}'（鍵 '{name}'）。僅支援 string/bool/整數/int64/double/float。");
		}

		return entry;
	}

	private sealed class GameplayVariablesDocumentDto
	{
		[JsonPropertyName("formatVersion")]
		public int FormatVersion { get; set; }

		[JsonPropertyName("entries")]
		public List<GameplayVariableEntryDto> Entries { get; set; } = new();
	}

	private sealed class GameplayVariableEntryDto
	{
		[JsonPropertyName("name")]
		public string Name { get; set; } = "";

		[JsonPropertyName("kind")]
		public string Kind { get; set; } = "";

		[JsonPropertyName("stringValue")]
		public string? StringValue { get; set; }

		[JsonPropertyName("boolValue")]
		public bool? BoolValue { get; set; }

		[JsonPropertyName("int64Value")]
		public long? Int64Value { get; set; }

		[JsonPropertyName("doubleValue")]
		public double? DoubleValue { get; set; }
	}
}
