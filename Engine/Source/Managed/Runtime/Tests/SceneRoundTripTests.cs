using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>场景往返测试。</summary>
public sealed class SceneRoundTripTests
{
	[Fact]
	public void CreateEntity_ReturnsValidEntity()
	{
		using var scene = SceneWorld.Create("TestScene");
		var entity = scene.CreateEntity("RoundTripEntity");

		Assert.NotNull(entity);
		Assert.True(entity.IsAlive);
		Assert.Equal(1, scene.EntityCount);
	}

	[Fact]
	public void SceneWorld_Serialize_Deserialize_RoundTrip()
	{
		using var scene1 = SceneWorld.Create("TestScene");
		scene1.CreateEntity("Entity1");
		scene1.CreateEntity("Entity2");

		// 序列化
		using var stream = new MemoryStream();
		scene1.Serialize(stream, "json");

		// 反序列化
		stream.Position = 0;
		using var scene2 = SceneWorld.Create("TestScene2");
		scene2.Deserialize(stream, "json");

		// 验证
		Assert.True(scene2.EntityCount > 0);
	}

	[Fact]
	public void SceneManager_LoadSave_RoundTrip()
	{
		var manager = new SceneManager();
		manager.LoadScene("TestScene");
		manager.CreateEntity("Entity1");

		// 保存到内存流
		var world = manager.GetWorld("TestScene");
		Assert.NotNull(world);

		using var stream = new MemoryStream();
		world!.Serialize(stream, "json");
		Assert.True(stream.Length > 0);

		// 从内存流加载
		stream.Position = 0;
		var manager2 = new SceneManager();
		manager2.LoadScene("TestScene2");
		var world2 = manager2.GetWorld("TestScene2");
		Assert.NotNull(world2);

		world2!.Deserialize(stream, "json");
		Assert.True(world2.EntityCount > 0);
	}
}
