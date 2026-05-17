using VisionGal.Managed.Serialization;
using Xunit;

namespace VisionGal.Managed.Foundation.Tests;

/// <summary>
/// 驗證 <see cref="VersionTolerance.CreateOptions"/> 下 JSON 反序列化對「未知欄位」之寬鬆行為，
/// 以利未來格式演進時舊客戶端仍可讀取核心欄位。
/// </summary>
public sealed class VersionToleranceTests
{
	[Fact]
	public void SceneDocument_Deserialize_IgnoresUnknownRootProperty()
	{
		const string json = """
			{
			  "formatVersion": 1,
			  "name": "FutureCompatScene",
			  "entities": [],
			  "futureSchemaField": { "reserved": true }
			}
			""";

		var doc = SceneSerializer.Deserialize(json);
		Assert.NotNull(doc);
		Assert.Equal(1, doc!.FormatVersion);
		Assert.Equal("FutureCompatScene", doc.Name);
		Assert.Empty(doc.Entities);
	}
}
