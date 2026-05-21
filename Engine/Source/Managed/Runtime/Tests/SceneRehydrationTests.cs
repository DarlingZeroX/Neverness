using Neverness.Editor.Framework.Serialization;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;
using SceneType = Neverness.Runtime.Scene.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>场景实体再水合与属性套用测试（Editor 序列化路径）。</summary>
public sealed class SceneRehydrationTests
{
	[Fact]
	public void ApplyEntryProperties_RestoresDisplayName()
	{
		var source = new SceneEntity(new NNEntityHandle(1), "SourceName");
		var entry = SceneSerializer.CaptureObject("Entity", source);

		var target = new SceneEntity(new NNEntityHandle(2), "Placeholder");
		SceneSerializer.ApplyEntryProperties(target, entry);

		Assert.Equal("SourceName", target.DisplayName);
	}

	[Fact]
	public void RestoreFromJsonWithEntities_RecreatesDisplayName_WhenEngineInstalled()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var entity = SceneEntity.Spawn("SceneEntity", "RehydratedDisplay");
		if (entity == null)
		{
			return;
		}

		var scene = new Scene.Scene("RehydrateScene");
		scene.AddEntity(entity);
		var doc = new SceneSerializer.SceneDocument { Name = scene.Name };
		doc.Entities.Add(SceneSerializer.CaptureObject(entity.DisplayName, entity));
		var json = SceneSerializer.Serialize(doc);

		var restored = SceneRehydrator.RestoreFromJsonWithEntities(json);
		Assert.NotNull(restored);
		Assert.Equal("RehydrateScene", restored!.Name);
		Assert.Single(restored.Entities);
		Assert.Equal("RehydratedDisplay", restored.Entities[0].DisplayName);
		Assert.True(restored.Entities[0].IsAlive);
	}
}
