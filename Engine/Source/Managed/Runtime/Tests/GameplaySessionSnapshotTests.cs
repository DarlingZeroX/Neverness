using System.Text.Json;
using Neverness.Managed.Interop;
using Neverness.Managed.Gameplay;
using Neverness.Managed.Object;
using SceneModel = Neverness.Managed.Scene.Scene;
using Neverness.Managed.Scene;
using Neverness.Managed.Serialization;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>Phase 6 slice 4：<see cref="GameplaySessionSnapshot"/> 根 JSON 與嵌套變子文檔。</summary>
public sealed class GameplaySessionSnapshotTests
{
	[Fact]
	public void RoundTrip_VariablesOnly_PreservesEntries()
	{
		var store = new GameplayVariableStore();
		store.Set("flag", true);
		store.Set("count", 42L);

		var snap = GameplaySessionSnapshot.Capture(store, sceneJson: null);
		var json = snap.ToJson();
		Assert.True(GameplaySessionSnapshot.TryParseFromJson(json, out var back) && back is not null);
		Assert.Null(back.SceneJson);

		var applied = new GameplayVariableStore();
		Assert.True(back.ApplyTo(applied));
		Assert.True((bool)applied.Get("flag")!);
		Assert.Equal(42L, applied.Get("count"));
	}

	[Fact]
	public void TryParseFromJson_IgnoresUnknownRootProperty()
	{
		var store = new GameplayVariableStore();
		store.Set("x", "y");
		var snap = GameplaySessionSnapshot.Capture(store, null);
		var json = snap.ToJson();

		using var doc = JsonDocument.Parse(json);
		var root = doc.RootElement.Clone();
		var wrapped = JsonSerializer.Serialize(
			new Dictionary<string, JsonElement>
			{
				["futureField"] = JsonSerializer.SerializeToElement(123, VersionTolerance.CreateOptions()),
				["formatVersion"] = root.GetProperty("formatVersion"),
				["variableStoreJson"] = root.GetProperty("variableStoreJson"),
			},
			VersionTolerance.CreateOptions());

		Assert.True(GameplaySessionSnapshot.TryParseFromJson(wrapped, out var parsed) && parsed is not null);
		var target = new GameplayVariableStore();
		Assert.True(parsed.ApplyTo(target));
		Assert.Equal("y", target.Get("x"));
	}

	[Fact]
	public void RoundTrip_WithSceneJson_PreservesScenePayload()
	{
		const string fakeScene = "{\"formatVersion\":1,\"name\":\"SnapScene\",\"entities\":[]}";

		var store = new GameplayVariableStore();
		store.Set("k", 1L);

		var snap = GameplaySessionSnapshot.Capture(store, fakeScene);
		var json = snap.ToJson();
		Assert.True(GameplaySessionSnapshot.TryParseFromJson(json, out var back) && back is not null);
		Assert.Equal(fakeScene, back.SceneJson);
	}

	[Fact]
	public void RoundTrip_WithLiveSceneJson_WhenEngineInstalled()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var entity = SceneEntity.Spawn("SceneEntity", "SnapEntity");
		if (entity == null)
		{
			return;
		}

		var scene = new SceneModel("SnapSceneModel");
		scene.AddEntity(entity);
		var sceneJson = scene.ToJson();
		entity.Destroy();

		var store = new GameplayVariableStore();
		store.Set("v", true);

		var snap = GameplaySessionSnapshot.Capture(store, sceneJson);
		var json = snap.ToJson();
		Assert.True(GameplaySessionSnapshot.TryParseFromJson(json, out var back) && back is not null);
		Assert.Equal(sceneJson, back.SceneJson);

		var appliedVars = new GameplayVariableStore();
		Assert.True(back.ApplyTo(appliedVars));
		Assert.True((bool)appliedVars.Get("v")!);
	}
}
