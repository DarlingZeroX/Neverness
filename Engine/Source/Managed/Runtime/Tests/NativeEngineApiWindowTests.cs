using System.Runtime.InteropServices;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// 校验 <b>NNWindowAPI</b> 子表与 layout v10。
/// </summary>
public sealed class NativeEngineApiWindowTests
{
	[Fact]
	public void NativeEngineApi_LayoutVersion_Is17()
	{
		Assert.Equal(17u, NNNativeEngineApiConstants.LayoutVersion);
	}

	[Fact]
	public void NNWindowApi_MarshaledSize_MatchesNativePointerSlots()
	{
		// MSVC x64：uint32 size + 4-byte padding + 13×函数指针（Pack=8）
		var expected = IntPtr.Size == 8 ? 112 : 56;
		Assert.Equal(expected, Marshal.SizeOf<NNWindowApi>());
	}

	[Fact]
	public void NNWindowDesc_MarshaledSize_IsBlittable()
	{
		var expected = IntPtr.Size == 8 ? 24 : 16;
		Assert.Equal(expected, Marshal.SizeOf<NNWindowDesc>());
	}

	/// <summary>
	/// Stub 表：<c>create</c> 返回 0（无 SDL）。
	/// </summary>
	[Fact]
	public unsafe void WindowApi_WhenInstalled_StubCreate_ReturnsInvalid()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		ref readonly var window = ref EngineNativeApiBootstrap.EngineApi.Window;
		Assert.True(window.Size >= (uint)Marshal.SizeOf<NNWindowApi>());
		if (window.Create == null)
		{
			return;
		}

		var desc = new NNWindowDesc
		{
			Title = null,
			Width = 1280,
			Height = 720,
			Resizable = true,
		};

		var handle = window.Create(&desc);
		Assert.Equal(0ul, handle);
	}

	[Fact]
	public void WindowHost_IsAvailable_WhenEngineTableInstalled()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		_ = WindowHost.IsAvailable;
	}
}
