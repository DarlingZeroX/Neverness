using Neverness.Runtime.Serialization;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>Runtime 场景二进制 ABI 薄封装占位测试。</summary>
public sealed class NNSceneSerializeBridgeTests
{
	[Fact]
	public void SerializeScene_ReturnsEmpty_WhenAbiNotWired()
	{
		var blob = NNSceneSerializeBridge.SerializeScene("TestScene");
		Assert.Empty(blob);
		Assert.False(NNSceneSerializeBridge.IsAvailable);
	}
}
