using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>資產 GUID 與匯入管線單元測試（無 CoreCLR Host）。</summary>
public sealed class GuidAndImportTests
{
	[Fact]
	public void FromDeterministicPath_IsStableAndNonZero()
	{
		var a = GUID.FromDeterministicPath("/assets/foo.png");
		var b = GUID.FromDeterministicPath("/assets/foo.png");
		var c = GUID.FromDeterministicPath("/assets/bar.png");

		Assert.False(a.IsZero);
		Assert.Equal(a, b);
		Assert.NotEqual(a, c);
	}

	[Fact]
	public void Import_WithoutNativeApi_UsesDeterministicGuid()
	{
		AssetDatabase.ClearForTesting();
		var path = "/assets/unit/test.vgtex";
		var guid = ImportPipeline.Import(new NVirtualPath(path) );

		Assert.False(guid.IsZero);
		Assert.True(AssetDatabase.TryResolveGuid(new NVirtualPath(path), out var resolved));
		Assert.Equal(guid, resolved);
		Assert.Equal(GUID.FromDeterministicPath(path), guid);
	}
}
