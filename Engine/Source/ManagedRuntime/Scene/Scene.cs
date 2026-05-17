using System.Text.Json;
using Neverness.Managed.Serialization;

namespace Neverness.Managed.Scene;

/// <summary>
/// 場景容器：實體清單與 JSON 往返序列化（DTO 層；不直接驅動 Native SceneAPI）。
/// </summary>
public sealed class Scene
{
	/// <summary>場景名稱。</summary>
	public string Name { get; set; }

	/// <summary>場景內實體。</summary>
	public List<SceneEntity> Entities { get; } = [];

	/// <summary>建立場景。</summary>
	/// <param name="name">非空白場景名稱。</param>
	public Scene(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		Name = name;
	}

	/// <summary>新增已建立之實體。</summary>
	public void AddEntity(SceneEntity entity)
	{
		ArgumentNullException.ThrowIfNull(entity);
		Entities.Add(entity);
	}

	/// <summary>序列化為 JSON 字串（含 <see cref="VersionTolerance.CurrentFormatVersion"/>）。</summary>
	public string ToJson()
	{
		var doc = new SceneSerializer.SceneDocument { Name = Name };
		foreach (var entity in Entities)
		{
			doc.Entities.Add(SceneSerializer.CaptureObject(entity.DisplayName, entity));
		}

		return SceneSerializer.Serialize(doc);
	}

	/// <summary>自 JSON 還原場景描述文件（不重建 <see cref="SceneEntity"/> 實例；僅 DTO）。</summary>
	public static SceneSerializer.SceneDocument? FromJson(string json) => SceneSerializer.Deserialize(json);

	/// <summary>
	/// 驗證 JSON 往返後之 DTO 是否與目前場景一致（名稱、實體數、首實體 <c>DisplayName</c> 屬性值）。
	/// </summary>
	/// <param name="json">由 <see cref="ToJson"/> 產生之 JSON。</param>
	/// <param name="expectedDisplayName">預期之首實體顯示名稱；為 null 時跳過屬性比對。</param>
	public bool ValidateRoundTripDocument(string json, string? expectedDisplayName = null)
	{
		var doc = FromJson(json);
		if (doc == null || doc.Name != Name || doc.Entities.Count != Entities.Count)
		{
			return false;
		}

		if (expectedDisplayName == null || Entities.Count == 0)
		{
			return true;
		}

		var entry = doc.Entities[0];
		if (!entry.Properties.TryGetValue(nameof(SceneEntity.DisplayName), out var propEl))
		{
			return false;
		}

		var roundTripName = propEl.ValueKind == JsonValueKind.String
			? propEl.GetString()
			: null;

		return roundTripName == expectedDisplayName;
	}

	/// <summary>
	/// 自場景描述文件還原僅託管層之 <see cref="Scene"/> 容器（不含實體；實體請用 <see cref="RehydrateFromJson"/> 或 <see cref="SceneRehydrator.RestoreFromDocumentWithEntities"/>）。
	/// </summary>
	public static Scene RestoreFromDocument(SceneSerializer.SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		return new Scene(document.Name);
	}

	/// <summary>
	/// 自 JSON 完整再水合場景（容器 + 經 <see cref="LifetimeSystem"/> 建立之實體與新 Native 控制代碼）。
	/// </summary>
	/// <param name="json">由 <see cref="ToJson"/> 產生之 JSON。</param>
	/// <returns>還原後場景；JSON 無效時回傳 null。</returns>
	public static Scene? RehydrateFromJson(string json) => SceneRehydrator.RestoreFromJsonWithEntities(json);
}
