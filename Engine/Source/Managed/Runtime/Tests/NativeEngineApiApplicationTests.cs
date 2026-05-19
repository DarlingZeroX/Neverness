using System.Runtime.InteropServices;
using Neverness.Managed.Engine;
using Neverness.Managed.Interop;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>
/// 校验 <b>NNApplicationAPI</b> 子表与 layout v6。
/// </summary>
public sealed class NativeEngineApiApplicationTests
{
	[Fact]
	public void NativeEngineApi_LayoutVersion_Is6()
	{
		Assert.Equal(6u, NNNativeEngineApiConstants.LayoutVersion);
	}

	[Fact]
	public void NNApplicationApi_MarshaledSize_MatchesNativePointerSlots()
	{
		// MSVC x64：uint32 size + 4-byte padding + 4×指针（Pack=8）
		var expected = IntPtr.Size == 8 ? 40 : 20;
		Assert.Equal(expected, Marshal.SizeOf<NNApplicationApi>());
	}

	[Fact]
	public unsafe void ApplicationApi_WhenInstalled_StubInitialize_ReturnsFalse()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		ref readonly var app = ref EngineNativeApiBootstrap.EngineApi.Application;
		Assert.True(app.Size >= (uint)Marshal.SizeOf<NNApplicationApi>());
		if (app.Initialize == null)
		{
			return;
		}

		Assert.False(app.Initialize());
	}
}
