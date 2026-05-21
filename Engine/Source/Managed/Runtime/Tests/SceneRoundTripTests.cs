using Neverness.Editor.Framework.Serialization;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using SceneType = Neverness.Runtime.Scene.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>场景 JSON 往返与 DTO 验证测试（Editor 序列化路径）。</summary>
public sealed class SceneRoundTripTests
{
	[Fact]
	public void CaptureObject_PreservesDisplayName()
	{
		var entity = new SceneEntity(new NNEntityHandle(1), "RoundTripEntity");
		var entry = SceneSerializer.CaptureObject(entity.DisplayName, entity);
		Assert.Equal("RoundTripEntity", entry.Properties[nameof(SceneEntity.DisplayName)].GetString());
	}

	[Fact]
	public void Serialize_WritesCurrentFormatVersion()
	{
		var doc = new SceneSerializer.SceneDocument { Name = "V" };
		var json = SceneSerializer.Serialize(doc);
		Assert.Contains("\"formatVersion\": 1", json);
	}

	[Fact]
	public void SceneDocument_RoundTrip_PreservesEntityDisplayName()
	{
		var entity = new SceneEntity(new NNEntityHandle(1), "RoundTripEntity");
		var doc = new SceneSerializer.SceneDocument { Name = "TestScene" };
		doc.Entities.Add(SceneSerializer.CaptureObject(entity.DisplayName, entity));

		var json = SceneSerializer.Serialize(doc);
		var restored = SceneSerializer.Deserialize(json);
		Assert.NotNull(restored);
		Assert.Equal("TestScene", restored!.Name);
		Assert.Single(restored.Entities);
		Assert.Equal("RoundTripEntity", restored.Entities[0].Properties[nameof(SceneEntity.DisplayName)].GetString());
	}
}
