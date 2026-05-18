using Neverness.Managed.Gameplay;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>Phase 6 Gameplay 變數表測試。</summary>
public sealed class GameplayVariableStoreTests
{
	[Fact]
	public void SetAndGet_RoundTrip()
	{
		var store = new GameplayVariableStore();
		store.Set("heroName", "Alice");
		Assert.Equal("Alice", store.Get("heroName"));
		Assert.True(store.TryGet("heroName", out var value));
		Assert.Equal("Alice", value);
	}

	[Fact]
	public void Remove_ClearsEntry()
	{
		var store = new GameplayVariableStore();
		store.Set("flag", true);
		store.Remove("flag");
		Assert.False(store.TryGet("flag", out _));
	}
}
