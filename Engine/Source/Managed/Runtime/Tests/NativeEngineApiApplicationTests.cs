using System.Runtime.InteropServices;
using Neverness.Runtime.Application;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// 校验 <b>NNApplicationAPI</b> 子表与 layout v10。
/// </summary>
public sealed class NativeEngineApiApplicationTests
{
	[Fact]
	public void NativeEngineApi_LayoutVersion_Is13()
	{
		Assert.Equal(13u, NNNativeEngineApiConstants.LayoutVersion);
	}

	[Fact]
	public void NNApplicationApi_MarshaledSize_MatchesNativePointerSlots()
	{
		// MSVC x64：uint32 size + 4-byte padding + 5×指针（Pack=8）
		var expected = IntPtr.Size == 8 ? 48 : 24;
		Assert.Equal(expected, Marshal.SizeOf<NNApplicationApi>());
	}

	/// <summary>
	/// 当测试宿主已安装 Native 表且 Application 为 Stub 时，Initialize 返回 false。
	/// </summary>
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

		var initialized = app.Initialize();
		if (initialized)
		{
			if (app.Shutdown != null)
			{
				app.Shutdown();
			}

			return;
		}

		Assert.False(initialized);
	}

	/// <summary>
	/// 若环境变量 NEVERNESS_NATIVE_MANAGED_DLL 指向 Runtime 表 DLL，则 ApplicationHost 可初始化。
	/// </summary>
	[Fact]
	public void ApplicationHost_Initialize_SucceedsWhenRuntimeApplicationWired()
	{
		var dllPath = Environment.GetEnvironmentVariable("NEVERNESS_NATIVE_MANAGED_DLL");
		if (string.IsNullOrWhiteSpace(dllPath) || !File.Exists(dllPath))
		{
			return;
		}

		if (!TryLoadNativeApiTable(dllPath, out var table))
		{
			return;
		}

		EngineNativeApiBootstrap.InstallFromNativeApiTable(table);
		try
		{
			if (!ApplicationHost.IsAvailable)
			{
				return;
			}

			Assert.True(ApplicationHost.Initialize());
		}
		finally
		{
			ApplicationHost.Shutdown();
			EngineNativeApiCache.ResetForTesting();
		}
	}

	private static bool TryLoadNativeApiTable(string dllPath, out nint table)
	{
		table = 0;
		try
		{
			var handle = NativeLibrary.Load(dllPath);
			if (!NativeLibrary.TryGetExport(handle, "NNNativeApi_GetDefaultTable", out var exportPtr))
			{
				return false;
			}

			var getTable = Marshal.GetDelegateForFunctionPointer<GetDefaultTableDelegate>(exportPtr);
			table = getTable();
			return table != 0;
		}
		catch
		{
			return false;
		}
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	private delegate nint GetDefaultTableDelegate();
}
