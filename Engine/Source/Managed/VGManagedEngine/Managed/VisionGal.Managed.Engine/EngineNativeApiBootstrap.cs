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
			// 與 Core 層一致：空指標不寫入快取。
			return;
		}

		if (p->ApiVersion != VGNativeApiConstants.ApiVersion)
		{
			// 主版本不符則不讀取 engineServices 指標，避免越界解讀。
			return;
		}

		if (p->EngineServices == 0)
		{
			// Phase 2 僅 Core 表之宿主：無引擎服務子表屬正常，此時不標記已安裝。
			return;
		}

		var pe = (VGNativeEngineApi*)p->EngineServices;
		if (pe->LayoutVersion != VGNativeEngineApiConstants.LayoutVersion)
		{
			// 引擎服務 ABI 佈局版本必須與託管端鏡像結構一致，否則拒絕快取。
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
			// 未通過佈局校驗時不觸碰函數指標，避免呼叫未定義位址。
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
