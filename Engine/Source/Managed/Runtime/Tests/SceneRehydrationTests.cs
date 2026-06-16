using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>场景实体再水合测试。</summary>
public sealed class SceneRehydrationTests
{
	[Fact]
	public void CreateEntity_ReturnsValidEntity()
	{
		using var scene = SceneWorld.Create("TestScene");
		var entity = scene.CreateEntity("TestEntity");

		Assert.NotNull(entity);
		Assert.True(entity.IsAlive);
	}

	[Fact]
	public void SceneManager_LoadScene_CreatesWorld()
	{
		var manager = new SceneManager();
		var result = manager.LoadScene("TestScene");

		Assert.True(result);
		Assert.True(manager.IsSceneLoaded("TestScene"));
		Assert.NotNull(manager.GetWorld("TestScene"));
	}

	[Fact]
	public void SceneManager_CreateEntity_ReturnsEntity()
	{
		var manager = new SceneManager();
		manager.LoadScene("TestScene");

		var entity = manager.CreateEntity("TestEntity");

		Assert.NotNull(entity);
		Assert.True(entity!.IsAlive);
	}
}
