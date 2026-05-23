using Neverness.Runtime.Serialization;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>Runtime 场景序列化 ABI 薄封装占位测试（VFS 路径模式）。</summary>
public sealed class NNSceneSerializeBridgeTests
{
	[Fact]
	public void SerializeScene_ReturnsInvalid_WhenAbiNotWired()
	{
		var result = NNSceneSerializeBridge.SerializeScene(1, "/assets/test.scene");
		Assert.NotEqual(Neverness.Runtime.Engine.NNSceneResult.Ok, result);
		Assert.False(NNSceneSerializeBridge.IsAvailable);
	}

	[Fact]
	public void DeserializeScene_ReturnsInvalid_WhenAbiNotWired()
	{
		var (result, handle) = NNSceneSerializeBridge.DeserializeScene("/assets/test.scene");
		Assert.NotEqual(Neverness.Runtime.Engine.NNSceneResult.Ok, result);
		Assert.Equal(0ul, handle);
	}

	[Fact]
	public void ExpectedBlobFormatVersion_Is2()
	{
		Assert.Equal(2, NNSceneSerializeBridge.ExpectedBlobFormatVersion);
	}
}
