using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// 校验 <b>NNVfsAPI</b> 子表与 layout v10。
/// </summary>
public sealed class NativeEngineApiVfsTests
{
	[Fact]
	public void NativeEngineApi_LayoutVersion_Is17()
	{
		Assert.Equal(17u, NNNativeEngineApiConstants.LayoutVersion);
	}

	[Fact]
	public void NNVfsApi_MarshaledSize_MatchesNativePointerSlots()
	{
		// MSVC x64：uint32 size + 4-byte padding + 7×指针（Pack=8）
		var expected = IntPtr.Size == 8 ? 64 : 32;
		Assert.Equal(expected, Marshal.SizeOf<NNVfsApi>());
	}

	/// <summary>
	/// Stub 表：<c>readText</c> 返回 0。
	/// </summary>
	[Fact]
	public unsafe void VfsApi_WhenInstalled_StubReadText_ReturnsFalse()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		ref readonly var vfs = ref EngineNativeApiBootstrap.EngineApi.Vfs;
		Assert.True(vfs.Size >= (uint)Marshal.SizeOf<NNVfsApi>());
		if (vfs.ReadText == null)
		{
			return;
		}

		byte* outText = null;
		var pathBytes = System.Text.Encoding.UTF8.GetBytes("test.txt\0");
		fixed (byte* path = pathBytes)
		{
			var ok = vfs.ReadText(path, &outText);
			Assert.Equal(0, ok);
			Assert.Equal(nint.Zero, (nint)outText);
		}
	}
}
