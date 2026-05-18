using Neverness.Managed.Reflection;
using Neverness.Managed.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>反射元資料掃描規則測試。</summary>
public sealed class TypeMetadataTests
{
	[Fact]
	public void SceneEntity_IncludesSerializeFieldDisplayName()
	{
		var meta = ReflectionRegistry.GetOrCreate(typeof(SceneEntity));
		Assert.Contains(meta.SerializableProperties, p => p.Name == nameof(SceneEntity.DisplayName));
	}

	private sealed class PropertyScanFixture
	{
		public string PublicAuto { get; set; } = "";

		[SerializeField]
		private string _privateField = "";

#pragma warning disable CS0414
		private string _ignored = "";
#pragma warning restore CS0414
	}

	[Fact]
	public void ScanMembers_RespectsSerializeFieldAndPublicSetter()
	{
		var meta = ReflectionRegistry.GetOrCreate(typeof(PropertyScanFixture));
		var names = meta.SerializableProperties.Select(p => p.Name).ToList();
		Assert.Contains("PublicAuto", names);
		Assert.Contains("_privateField", names);
		Assert.DoesNotContain("_ignored", names);
	}
}
