using Neverness.Managed.Engine;
using Neverness.Managed.Serialization;
using SceneType = Neverness.Managed.Scene.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>场景 JSON 往返与 DTO 验证测试。</summary>
public sealed class SceneRoundTripTests
{
	[Fact]
	public void ValidateRoundTripDocument_PreservesDisplayName()
	{
		var entity = new Neverness.Managed.Scene.SceneEntity(new NNEntityHandle(1), "RoundTripEntity");
		var scene = new SceneType("TestScene");
		scene.AddEntity(entity);

		var json = scene.ToJson();
		Assert.True(scene.ValidateRoundTripDocument(json, "RoundTripEntity"));
	}

	[Fact]
	public void Serialize_WritesCurrentFormatVersion()
	{
		var doc = new SceneSerializer.SceneDocument { Name = "V" };
		var json = SceneSerializer.Serialize(doc);
		Assert.Contains("\"formatVersion\": 1", json);
	}

	[Fact]
	public void RestoreFromDocument_PreservesName()
	{
		var doc = new SceneSerializer.SceneDocument { Name = "Restored" };
		var scene = SceneType.RestoreFromDocument(doc);
		Assert.Equal("Restored", scene.Name);
	}
}
