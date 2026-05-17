using VisionGal.Managed.Gameplay;

namespace VisionGal.Managed.Foundation.Tests;

/// <summary>Phase 6：<see cref="GameplayVariableStore"/> JSON 快照往返。</summary>
public sealed class GameplayVariableStoreJsonTests
{
	[Fact]
	public void ToJson_TryParseFromJson_RoundTrip_PrimitiveKinds()
	{
		var store = new GameplayVariableStore();
		store.Set("s", "hello");
		store.Set("b", true);
		store.Set("i", 42L);
		store.Set("d", 3.25);

		var json = store.ToJson();
		Assert.True(GameplayVariableStore.TryParseFromJson(json, out var copy));
		Assert.NotNull(copy);
		Assert.Equal("hello", copy!.Get("s"));
		Assert.Equal(true, copy.Get("b"));
		Assert.Equal(42L, copy.Get("i"));
		Assert.Equal(3.25, copy.Get("d"));
	}

	[Fact]
	public void TryParseFromJson_InvalidKind_ReturnsFalse()
	{
		const string bad = /*lang=json,strict*/ """
			{
			  "formatVersion": 1,
			  "entries": [
			    { "name": "x", "kind": "unknown", "stringValue": "a" }
			  ]
			}
			""";
		Assert.False(GameplayVariableStore.TryParseFromJson(bad, out var store));
		Assert.Null(store);
	}
}
