using Neverness.Managed.Assets;
using Neverness.Managed.Interop;
using Neverness.Managed.Object;
using Neverness.Managed.Scene;
using SceneModel = Neverness.Managed.Scene.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>
/// 原 <c>Entry.BootstrapEngineFoundation</c> 演练：Object / Scene / Assets，不依赖 UCO。
/// </summary>
public sealed class FoundationBootstrapDrillTests : IDisposable
{
	public FoundationBootstrapDrillTests()
	{
		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();
	}

	public void Dispose()
	{
		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();
	}

	[Fact]
	public void Foundation_drill_object_scene_assets_roundtrip()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var entity = SceneEntity.Spawn("SceneEntity", "BootstrapEntity");
		if (entity == null)
		{
			return;
		}

		var nativeSceneEntityOk = entity.IsAlive;

		var scene = new SceneModel("BootstrapScene");
		scene.AddEntity(entity);

		var json = scene.ToJson();
		var sceneRoundTripOk = scene.ValidateRoundTripDocument(json, entity.DisplayName);

		var expectedDisplayName = entity.DisplayName;
		entity.Destroy();
		AssetDatabase.ClearForTesting();

		var rehydratedScene = SceneRehydrator.RestoreFromJsonWithEntities(json);
		var rehydratedEntity = rehydratedScene?.Entities.Count == 1 ? rehydratedScene.Entities[0] : null;
		var sceneRehydrationOk =
			rehydratedScene != null &&
			rehydratedScene.Name == "BootstrapScene" &&
			rehydratedEntity != null &&
			rehydratedEntity.DisplayName == expectedDisplayName &&
			rehydratedEntity.IsAlive;

		const string assetPath = "/assets/bootstrap/test.png";
		var imported = ImportPipeline.Import(assetPath);
		var assetOk = !imported.IsZero && AssetDatabase.TryResolveGuid(assetPath, out var resolved) && resolved == imported;

		Assert.True(nativeSceneEntityOk && sceneRoundTripOk && sceneRehydrationOk && assetOk);
	}
}
