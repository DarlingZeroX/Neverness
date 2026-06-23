using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Framework.Serialization;

/// <summary>
/// 场景 DTO → Native 实体再水合（Editor/工具链）。
/// 对齐新 ABI：SceneManager 驱动生命周期，实体经 CreateEntity + AddComponent 创建。
/// </summary>
public static class SceneRehydrator
{
    /// <summary>自 JSON 还原完整场景。</summary>
    public static SceneManager? RestoreFromJsonWithEntities(string json)
    {
        var doc = SceneSerializer.Deserialize(json);
        return doc == null ? null : RestoreFromDocumentWithEntities(doc);
    }

    /// <summary>自场景描述文件经 Native API 再水合。</summary>
    public static SceneManager RestoreFromDocumentWithEntities(SceneSerializer.SceneDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);

        var manager = new SceneManager();
        if (EngineNativeApiBootstrap.IsInstalled)
        {
            _ = manager.LoadScene(document.Name);
        }

        foreach (var entry in document.Entities)
        {
            var entity = CreateEntityFromEntry(manager, entry);
        }

        return manager;
    }

    private static SceneEntity? CreateEntityFromEntry(SceneManager manager, SceneSerializer.SceneEntityEntry entry)
    {
        ArgumentNullException.ThrowIfNull(entry);

        if (!EngineNativeApiBootstrap.IsInstalled || !manager.HasActiveScene)
        {
            return null;
        }

        var entity = manager.CreateEntity(entry.Name);
        if (entity == null)
        {
            return null;
        }

        SceneSerializer.ApplyEntryProperties(entity, entry);
        return entity;
    }
}
