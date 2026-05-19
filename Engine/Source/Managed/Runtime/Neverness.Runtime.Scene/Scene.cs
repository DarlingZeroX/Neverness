using System.Text.Json;
using Neverness.Managed.Serialization;

namespace Neverness.Managed.Scene;

/// <summary>
/// 场景门面：运行时经 <see cref="SceneNativeBridge"/> 访问 Native 场景图；JSON 为工具/存档 DTO 路径。
/// </summary>
public sealed class Scene
{
	/// <summary>场景名称。</summary>
	public string Name { get; set; }

	/// <summary>本 Managed 会话跟踪的实体（Native 为权威存储）。</summary>
	public List<SceneEntity> Entities { get; } = [];

	/// <summary>建立场景。</summary>
	public Scene(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		Name = name;
	}

	/// <summary>经 Native <c>loadScene</c> 加载本场景名。</summary>
	public int LoadNative()
	{
		var result = SceneNativeBridge.LoadScene(Name);
		return result;
	}

	/// <summary>经 Native <c>unloadScene</c> 卸载本场景名。</summary>
	public int UnloadNative() => SceneNativeBridge.UnloadScene(Name);

	/// <summary>经 Native spawn 生成实体并加入跟踪列表。</summary>
	public SceneEntity? SpawnEntity(string prefabVirtualPath, string displayName = "Entity")
	{
		var entity = SceneEntity.Spawn(prefabVirtualPath, displayName);
		if (entity != null)
		{
			Entities.Add(entity);
		}

		return entity;
	}

	/// <summary>登记已存在的实体门面。</summary>
	public void AddEntity(SceneEntity entity)
	{
		ArgumentNullException.ThrowIfNull(entity);
		Entities.Add(entity);
	}

	/// <summary>序列化为 JSON（含实体可序列化属性快照）。</summary>
	public string ToJson()
	{
		var doc = new SceneSerializer.SceneDocument { Name = Name };
		foreach (var entity in Entities)
		{
			doc.Entities.Add(SceneSerializer.CaptureObject(entity.DisplayName, entity));
		}

		return SceneSerializer.Serialize(doc);
	}

	/// <summary>自 JSON 还原场景描述文件（不重建实体；仅 DTO）。</summary>
	public static SceneSerializer.SceneDocument? FromJson(string json) => SceneSerializer.Deserialize(json);

	/// <summary>验证 JSON 往返 DTO 是否与当前场景一致。</summary>
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

	/// <summary>自 DTO 还原仅托管容器（不含实体）。</summary>
	public static Scene RestoreFromDocument(SceneSerializer.SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);
		return new Scene(document.Name);
	}

	/// <summary>自 JSON 经 Native spawn 再水合（需已安装 Engine Scene API）。</summary>
	public static Scene? RehydrateFromJson(string json) => SceneRehydrator.RestoreFromJsonWithEntities(json);
}
