using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Framework.Serialization;

/// <summary>
/// 场景 DTO → Native 实体再水合（Editor/工具链）。
/// </summary>
public static class SceneRehydrator
{
	/// <summary>自 JSON 还原完整场景。</summary>
	public static Scene? RestoreFromJsonWithEntities(string json)
	{
		var doc = SceneSerializer.Deserialize(json);
		return doc == null ? null : RestoreFromDocumentWithEntities(doc);
	}

	/// <summary>自场景描述文件经 Native API 再水合。</summary>
	public static Scene RestoreFromDocumentWithEntities(SceneSerializer.SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);

		var scene = new Scene(document.Name);
		if (EngineNativeApiBootstrap.IsInstalled)
		{
			_ = scene.LoadNative();
		}

		foreach (var entry in document.Entities)
		{
			var entity = CreateEntityFromEntry(entry);
			if (entity != null)
			{
				scene.AddEntity(entity);
			}
		}

		return scene;
	}

	private static SceneEntity? CreateEntityFromEntry(SceneSerializer.SceneEntityEntry entry)
	{
		ArgumentNullException.ThrowIfNull(entry);

		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return null;
		}

		var prefabPath = ResolvePrefabPath(entry);
		var entity = SceneEntity.Spawn(prefabPath);
		if (entity == null)
		{
			return null;
		}

		SceneSerializer.ApplyEntryProperties(entity, entry);
		return entity;
	}

	private static string ResolvePrefabPath(SceneSerializer.SceneEntityEntry entry)
	{
		if (!string.IsNullOrWhiteSpace(entry.TypeName))
		{
			var resolved = Type.GetType(entry.TypeName, throwOnError: false);
			if (resolved != null)
			{
				return resolved.Name;
			}
		}

		return string.IsNullOrWhiteSpace(entry.Name) ? nameof(SceneEntity) : entry.Name;
	}
}
