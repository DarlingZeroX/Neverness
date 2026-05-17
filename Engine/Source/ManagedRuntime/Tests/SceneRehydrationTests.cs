using Neverness.Managed.Engine;
using Neverness.Managed.Object;
using Neverness.Managed.Serialization;
using SceneType = Neverness.Managed.Scene.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>場景實體再水合與屬性套用測試。</summary>
public sealed class SceneRehydrationTests : IDisposable
{
	public SceneRehydrationTests()
	{
		ObjectRegistry.ClearForTesting();
	}

	public void Dispose()
	{
		ObjectRegistry.ClearForTesting();
	}

	[Fact]
	public void ApplyEntryProperties_RestoresDisplayName()
	{
		var source = new Neverness.Managed.Scene.SceneEntity(new VGObjectId(1), new VGObjectHandle(1), "SourceName");
		var entry = SceneSerializer.CaptureObject("Entity", source);

		var target = new Neverness.Managed.Scene.SceneEntity(new VGObjectId(2), new VGObjectHandle(2), "Placeholder");
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

		var entity = LifetimeSystem.CreateAndRegister<Neverness.Managed.Scene.SceneEntity>("RehydrateTest");
		entity.DisplayName = "RehydratedDisplay";

		var scene = new SceneType("RehydrateScene");
		scene.AddEntity(entity);
		var json = scene.ToJson();

		ObjectRegistry.ClearForTesting();

		var restored = SceneType.RehydrateFromJson(json);
		Assert.NotNull(restored);
		Assert.Equal("RehydrateScene", restored.Name);
		Assert.Single(restored.Entities);
		Assert.Equal("RehydratedDisplay", restored.Entities[0].DisplayName);
		Assert.True(NativeHandleBridge.IsAlive(restored.Entities[0].Handle));
	}
}
