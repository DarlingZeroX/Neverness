using Neverness.Runtime.Serialization;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>Runtime 场景序列化桥接测试。</summary>
public sealed class NNSceneSerializeBridgeTests
{
	[Fact]
	public void SerializeScene_ThrowsException_WhenNoScene()
	{
		Assert.Throws<ArgumentNullException>(() =>
			NNSceneSerializeBridge.SerializeScene(null!, "/assets/test.scene"));
	}

	[Fact]
	public void DeserializeScene_ReturnsWorld_WhenNoFile()
	{
		// 注意：LoadFromAsset 在文件不存在时仍然返回一个空的 SceneWorld
		var result = NNSceneSerializeBridge.DeserializeScene("TestScene", "/nonexistent/test.scene");
		Assert.NotNull(result);
		Assert.Equal(0, result!.EntityCount);
	}

	[Fact]
	public void ExpectedBlobFormatVersion_Is2()
	{
		Assert.Equal(2, NNSceneSerializeBridge.ExpectedBlobFormatVersion);
	}
}
