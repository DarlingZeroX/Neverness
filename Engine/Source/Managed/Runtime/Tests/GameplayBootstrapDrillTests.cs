using Neverness.Managed.Assets;
using Neverness.Managed.Gameplay;
using Neverness.Managed.Interop;
using Neverness.Managed.Object;
using Neverness.Managed.Scene;
using SceneModel = Neverness.Managed.Scene.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>
/// 原 <c>Entry.BootstrapGameplay</c> 演练（含 Session 快照），不依赖 UCO。
/// </summary>
public sealed class GameplayBootstrapDrillTests : IDisposable
{
	public GameplayBootstrapDrillTests()
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
	public void Gameplay_drill_variable_sequence_session_snapshot()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var store = new GameplayVariableStore();
		store.Set("gpBootstrap", false);

		var json = store.ToJson();
		Assert.True(GameplayVariableStore.TryParseFromJson(json, out var roundTrip) && roundTrip is not null);
		Assert.True(roundTrip.TryGet("gpBootstrap", out var v0) && v0 is bool b0 && !b0);

		const string expectedRehydratedTitle = "GameplayBootstrapEntity";
		var preloadEntity = SceneEntity.Spawn("SceneEntity", expectedRehydratedTitle);
		if (preloadEntity == null)
		{
			return;
		}

		var preloadScene = new SceneModel("GameplayBootstrapScene");
		preloadScene.AddEntity(preloadEntity);
		var sceneJson = preloadScene.ToJson();
		preloadEntity.Destroy();
		AssetDatabase.ClearForTesting();

		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new RehydrateSceneSequenceStep(sceneJson),
				new SyncFirstEntityDisplayNameToVariableSequenceStep("rehydratedTitle"),
				new SetVariableSequenceStep("gpBootstrap", true),
				new PresentDialogueSequenceStep(0, "GameplayBootstrapDrillTests"),
			});

		Assert.True(runner.Run(store));
		Assert.True((bool)store.Get("gpBootstrap")!);
		Assert.Equal(expectedRehydratedTitle, store.Get("rehydratedTitle"));

		var snap = GameplaySessionSnapshot.Capture(store, sceneJson);
		var snapJson = snap.ToJson();
		Assert.True(GameplaySessionSnapshot.TryParseFromJson(snapJson, out var parsed) && parsed is not null);
		Assert.Equal(sceneJson, parsed.SceneJson);
	}
}
