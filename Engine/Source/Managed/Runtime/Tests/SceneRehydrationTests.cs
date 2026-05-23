using Neverness.Editor.Framework.Serialization;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>场景实体再水合与属性套用测试（Editor 序列化路径）。</summary>
public sealed class SceneRehydrationTests
{
	[Fact]
	public void ApplyEntryProperties_RestoresDisplayName()
	{
		var source = new SceneEntity(new NNEntityHandle(1), displayName: "SourceName");
		var entry = SceneSerializer.CaptureObject("Entity", source);

		var target = new SceneEntity(new NNEntityHandle(2), displayName: "Placeholder");
		SceneSerializer.ApplyEntryProperties(target, entry);

		Assert.Equal("SourceName", target.DisplayName);
	}

	[Fact]
	public void RestoreFromJsonWithEntities_RecreatesSceneManager_WhenEngineInstalled()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var manager = new SceneManager();
		_ = manager.LoadScene("RehydrateScene");
		var entity = manager.CreateEntity("RehydratedDisplay");
		if (entity == null)
		{
			return;
		}

		var doc = new SceneSerializer.SceneDocument { Name = "RehydrateScene" };
		doc.Entities.Add(SceneSerializer.CaptureObject(entity.DisplayName, entity));
		var json = SceneSerializer.Serialize(doc);

		var restored = SceneRehydrator.RestoreFromJsonWithEntities(json);
		Assert.NotNull(restored);
		Assert.True(restored!.IsSceneLoaded("RehydrateScene"));
		Assert.Single(restored.Entities);
		Assert.Equal("RehydratedDisplay", restored.Entities[0].DisplayName);
	}
}
