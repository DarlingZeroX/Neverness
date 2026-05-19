using Neverness.Managed.Engine;
using Neverness.Managed.Interop;
using Neverness.Managed.Scene;
using Neverness.Managed.Serialization;
using SceneType = Neverness.Managed.Scene.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>场景实体再水合与属性套用测试。</summary>
public sealed class SceneRehydrationTests
{
	[Fact]
	public void ApplyEntryProperties_RestoresDisplayName()
	{
		var source = new Neverness.Managed.Scene.SceneEntity(new NNEntityHandle(1), "SourceName");
		var entry = SceneSerializer.CaptureObject("Entity", source);

		var target = new Neverness.Managed.Scene.SceneEntity(new NNEntityHandle(2), "Placeholder");
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

		var scene = new SceneType("RehydrateScene");
		scene.AddEntity(entity);
		var json = scene.ToJson();

		var restored = SceneType.RehydrateFromJson(json);
		Assert.NotNull(restored);
		Assert.Equal("RehydrateScene", restored.Name);
		Assert.Single(restored.Entities);
		Assert.Equal("RehydratedDisplay", restored.Entities[0].DisplayName);
		Assert.True(restored.Entities[0].IsAlive);
	}
}
