using Neverness.Editor.Framework.Reflection;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>Editor 反射元数据扫描测试。</summary>
public sealed class TypeMetadataTests
{
	[Fact]
	public void SceneEntity_IncludesDisplayNameProperty()
	{
		var meta = ReflectionRegistry.GetOrCreate(typeof(SceneEntity));
		Assert.Contains(meta.SerializableProperties, p => p.Name == nameof(SceneEntity.DisplayName));
	}

	private sealed class PropertyScanFixture
	{
		[SerializeField]
		private int _hiddenField;

		public string PublicProp { get; set; } = string.Empty;
	}

	[Fact]
	public void ScanMembers_RespectsSerializeFieldAndPublicSetter()
	{
		var meta = ReflectionRegistry.GetOrCreate(typeof(PropertyScanFixture));
		Assert.Contains(meta.SerializableProperties, p => p.Name == nameof(PropertyScanFixture.PublicProp));
	}
}
