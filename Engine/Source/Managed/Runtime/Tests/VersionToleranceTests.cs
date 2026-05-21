using Neverness.Editor.Framework.Serialization;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// 验证 Editor <see cref="VersionTolerance.CreateOptions"/> 下 JSON 反序列化对未知字段的宽松行为。
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
