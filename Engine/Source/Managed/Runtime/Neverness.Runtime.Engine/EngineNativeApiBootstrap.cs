using System.Runtime.InteropServices;
using Neverness.Managed.Core;

namespace Neverness.Managed.Engine;

/// <summary>
/// 從 <c>NNNativeAPI.engineServices</c> 安裝 **Engine Service** 函數表鏡像（按值複製函數指標，與 <see cref="NativeApiBootstrap"/> 策略一致）。
/// 執行緒：僅允許在 Bootstrap 階段由宿主單執行緒呼叫；之後視為唯讀快照。
/// </summary>
/// <remarks>
/// **layout v5** 起含 **<see cref="NNEntityApi"/>**（**<c>GetRuntimeTick</c>** 等）；與託管 **EntityWorld** 無自動資料同步（見 MANAGED 總覽 **§2.7.1**）。若 Native **<c>layoutVersion</c>** 與 <see cref="NNNativeEngineApiConstants.LayoutVersion"/> 不一致，本類不會標記 **<see cref="IsInstalled"/>**。
/// </remarks>
public static unsafe class EngineNativeApiBootstrap
{
	private static NNNativeEngineApi s_engineApi;
	private static volatile bool s_installed;

	/// <summary>是否已成功校驗 <c>layoutVersion</c> 並快取子表。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>取得已安裝之引擎服務表唯讀參考；若未安裝則回傳預設結構（全空指標）。</summary>
	public static ref readonly NNNativeEngineApi EngineApi => ref s_engineApi;

	/// <summary>
	/// 由 <c>NNNativeAPI*</c> 指標（以 <see cref="nint"/> 傳遞）解析並安裝 <c>engineServices</c>。
	/// </summary>
	/// <param name="nativeApiTable">與 <see cref="NativeApiBootstrap.Install"/> 相同來源之 Native 表指標。</param>
	public static void InstallFromNativeApiTable(nint nativeApiTable)
	{
		var p = (NNNativeApi*)nativeApiTable;
		if (p == null)
		{
			// 與 Core 層一致：空指標不寫入快取。
			return;
		}

		if (p->ApiVersion != NNNativeApiConstants.ApiVersion)
		{
			// 主版本不符則不讀取 engineServices 指標，避免越界解讀。
			return;
		}

		if (p->EngineServices == 0)
		{
			// Phase 2 僅 Core 表之宿主：無引擎服務子表屬正常，此時不標記已安裝。
			return;
		}

		var pe = (NNNativeEngineApi*)p->EngineServices;
		if (pe->LayoutVersion != NNNativeEngineApiConstants.LayoutVersion)
		{
			// 引擎服務 ABI 佈局版本必須與託管端鏡像結構一致，否則拒絕快取。
			return;
		}

		s_engineApi = *pe;
		s_installed = true;
	}

	/// <summary>
	/// 供測試與宿主驗證：經函數表間接呼叫 Stub（**Timing**、**AsyncWait**），並在 **layout v5** 下對 **<c>NNEntityAPI</c>**（**<c>getServiceAbiToken</c>** 魔數、可選 **<c>getRuntimeTick</c>**）做 ABI 冒煙。
	/// </summary>
	/// <remarks>
	/// 不配置 GPU／音訊等真實資源；**Entity** 路徑不表示 <c>EntityWorld</c> 已與 Native 資料鏡像（見總覽 MANAGED **§2.7.1** 與 **§207** 殘留策略）。
	/// </remarks>
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

		// layout v5：NNEntityAPI 冒煙（與 EntityWorld 無關；僅驗證函數指標與魔數與 Native 一致）。
		if (s_engineApi.Entity.GetServiceAbiToken != null)
		{
			var token = s_engineApi.Entity.GetServiceAbiToken();
			if (token != NNNativeEngineApiConstants.EntityServiceAbiToken)
			{
				// 與 Engine 表校驗策略一致：不拋例外以免破壞宿主啟動；測試應以斷言覆蓋期望值。
				return;
			}
		}

		// getRuntimeTick：Runtime 覆寫後隨宿主 Tick 遞增；純 dotnet 測試未注入表時指標可為 null。
		if (s_engineApi.Entity.GetRuntimeTick != null)
		{
			_ = s_engineApi.Entity.GetRuntimeTick();
		}
	}
}
