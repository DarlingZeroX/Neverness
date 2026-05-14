using System.Runtime.InteropServices;
using VisionGal.Managed.Core;

namespace VisionGal.Managed.Engine;

/// <summary>
/// 從 <c>VGNativeAPI.engineServices</c> 安裝 **Engine Service** 函數表鏡像（按值複製函數指標，與 <see cref="NativeApiBootstrap"/> 策略一致）。
/// 執行緒：僅允許在 Bootstrap 階段由宿主單執行緒呼叫；之後視為唯讀快照。
/// </summary>
public static unsafe class EngineNativeApiBootstrap
{
	private static VGNativeEngineApi s_engineApi;
	private static volatile bool s_installed;

	/// <summary>是否已成功校驗 <c>layoutVersion</c> 並快取子表。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>取得已安裝之引擎服務表唯讀參考；若未安裝則回傳預設結構（全空指標）。</summary>
	public static ref readonly VGNativeEngineApi EngineApi => ref s_engineApi;

	/// <summary>
	/// 由 <c>VGNativeAPI*</c> 指標（以 <see cref="nint"/> 傳遞）解析並安裝 <c>engineServices</c>。
	/// </summary>
	/// <param name="nativeApiTable">與 <see cref="NativeApiBootstrap.Install"/> 相同來源之 Native 表指標。</param>
	public static void InstallFromNativeApiTable(nint nativeApiTable)
	{
		var p = (VGNativeApi*)nativeApiTable;
		if (p == null)
		{
			return;
		}

		if (p->ApiVersion != VGNativeApiConstants.ApiVersion)
		{
			return;
		}

		if (p->EngineServices == 0)
		{
			return;
		}

		var pe = (VGNativeEngineApi*)p->EngineServices;
		if (pe->LayoutVersion != VGNativeEngineApiConstants.LayoutVersion)
		{
			return;
		}

		s_engineApi = *pe;
		s_installed = true;
	}

	/// <summary>
	/// 供測試與宿主驗證：經函數表間接呼叫 Stub（GetDeltaTime + AsyncWait 路徑），不配置真實引擎資源。
	/// </summary>
	public static void ExerciseStubInteropPath()
	{
		if (!s_installed)
		{
			return;
		}

		if (s_engineApi.Timing.GetDeltaTime != null)
		{
			_ = s_engineApi.Timing.GetDeltaTime();
		}

		if (s_engineApi.Timing.GetTotalTime != null)
		{
			_ = s_engineApi.Timing.GetTotalTime();
		}

		if (s_engineApi.Timing.GetFrameIndex != null)
		{
			_ = s_engineApi.Timing.GetFrameIndex();
		}

		if (s_engineApi.AsyncWait.CreateWait != null &&
		    s_engineApi.AsyncWait.TryComplete != null &&
		    s_engineApi.AsyncWait.ReleaseWait != null)
		{
			var h = s_engineApi.AsyncWait.CreateWait();
			_ = s_engineApi.AsyncWait.TryComplete(h);
			s_engineApi.AsyncWait.ReleaseWait(h);
		}
	}
}
