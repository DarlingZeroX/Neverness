using System.Diagnostics.CodeAnalysis;
using System.Text.Json;
using System.Text.Json.Serialization;
using VisionGal.Managed.Serialization;

namespace VisionGal.Managed.Gameplay;

/// <summary>
/// Phase 6 slice 4：託管「會話／存檔快照」根 DTO——在單一 JSON 文件中組裝 <b>變數表 JSON 字串</b>（語意與 <see cref="GameplayVariableStore.ToJson"/> 一致）與可選的 <b>場景 JSON 字串</b>（語意與 <see cref="T:VisionGal.Managed.Scene.Scene.ToJson"/> 一致）。
/// </summary>
/// <remarks>
/// <para>設計要點：不重複定義變數條目或場景實體之內部欄位，僅做容器與 <c>formatVersion</c>，降低與 <see cref="VisionGal.Managed.Serialization.SceneSerializer"/> 等模組漂移風險。</para>
/// <para>序列化選項與全專案一致：<see cref="VersionTolerance.CreateOptions()"/>（寬鬆讀取、可忽略未知根欄位）。</para>
/// <para><b>與 Native 存檔路線之關係</b>：當前僅託管 JSON；未來若在 <b>VGNativeEngineAPI</b> 納入 Gameplay／存檔服務表，可將本 DTO 之 <see cref="ToJson"/> 輸出作為寫入檔案之內容來源，或由 Native 僅負責路徑／slot 與錯誤碼（見總覽 <b>MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md</b> 之 <c>§5.1</c> 草案）。</para>
/// </remarks>
public sealed class GameplaySessionSnapshot
{
	/// <summary>與 <see cref="VersionTolerance.CurrentFormatVersion"/> 對齊之根格式版本。</summary>
	public int FormatVersion { get; init; }

	/// <summary>
	/// 嵌套之變數表 JSON 文字（UTF-16）；必須能被 <see cref="GameplayVariableStore.TryParseFromJson"/> 成功解析。
	/// </summary>
	public string VariableStoreJson { get; init; } = "";

	/// <summary>可選之場景 JSON 文字；無場景時為 null 或省略寫入。</summary>
	public string? SceneJson { get; init; }

	/// <summary>
	/// 由當前變數表與可選場景 JSON 建立快照（變數部分立即呼叫 <see cref="GameplayVariableStore.ToJson"/>）。
	/// </summary>
	/// <param name="variables">非空之變數表。</param>
	/// <param name="sceneJson">場景 JSON；無則傳 null。</param>
	public static GameplaySessionSnapshot Capture(GameplayVariableStore variables, string? sceneJson)
	{
		ArgumentNullException.ThrowIfNull(variables);
		return new GameplaySessionSnapshot
		{
			FormatVersion = VersionTolerance.CurrentFormatVersion,
			VariableStoreJson = variables.ToJson(),
			SceneJson = string.IsNullOrWhiteSpace(sceneJson) ? null : sceneJson,
		};
	}

	/// <summary>將本快照序列化為根 JSON 字串（含 <c>formatVersion</c>）。</summary>
	public string ToJson()
	{
		var dto = new SessionSnapshotDocumentDto
		{
			FormatVersion = FormatVersion,
			VariableStoreJson = VariableStoreJson,
			SceneJson = SceneJson,
		};

		return JsonSerializer.Serialize(dto, VersionTolerance.CreateOptions());
	}

	/// <summary>
	/// 自根 JSON 還原快照；成功時校驗根版本與嵌套之 <see cref="VariableStoreJson"/> 可解析為變數表。
	/// </summary>
	/// <param name="json">UTF-16 JSON 文字。</param>
	/// <param name="snapshot">輸出快照；失敗時為 null。</param>
	/// <returns>根版本合法且變子文檔可解析時為 true。</returns>
	public static bool TryParseFromJson(ReadOnlySpan<char> json, [NotNullWhen(true)] out GameplaySessionSnapshot? snapshot)
	{
		snapshot = null;
		SessionSnapshotDocumentDto? doc;
		try
		{
			doc = JsonSerializer.Deserialize<SessionSnapshotDocumentDto>(json, VersionTolerance.CreateOptions());
		}
		catch (JsonException)
		{
			return false;
		}

		if (doc is null || string.IsNullOrWhiteSpace(doc.VariableStoreJson))
		{
			return false;
		}

		if (doc.FormatVersion < 1 || doc.FormatVersion > VersionTolerance.CurrentFormatVersion)
		{
			return false;
		}

		if (!GameplayVariableStore.TryParseFromJson(doc.VariableStoreJson.AsSpan(), out var validatedStore) || validatedStore is null)
		{
			return false;
		}

		snapshot = new GameplaySessionSnapshot
		{
			FormatVersion = doc.FormatVersion,
			VariableStoreJson = doc.VariableStoreJson,
			SceneJson = string.IsNullOrWhiteSpace(doc.SceneJson) ? null : doc.SceneJson,
		};

		return true;
	}

	/// <summary>
	/// 將快照中的變數子文檔還原並覆寫寫入 <paramref name="target"/>（先清空再以 <see cref="GameplayVariableStore.CopyFrom"/> 複製）。
	/// </summary>
	/// <param name="target">目標變數表，不可為 null。</param>
	/// <returns>嵌套變數 JSON 解析成功且已複製時為 true。</returns>
	public bool ApplyTo(GameplayVariableStore target)
	{
		ArgumentNullException.ThrowIfNull(target);
		if (!GameplayVariableStore.TryParseFromJson(VariableStoreJson.AsSpan(), out var parsed) || parsed is null)
		{
			return false;
		}

		target.CopyFrom(parsed);
		return true;
	}

	private sealed class SessionSnapshotDocumentDto
	{
		[JsonPropertyName("formatVersion")]
		public int FormatVersion { get; set; }

		[JsonPropertyName("variableStoreJson")]
		public string VariableStoreJson { get; set; } = "";

		[JsonPropertyName("sceneJson")]
		public string? SceneJson { get; set; }
	}
}
