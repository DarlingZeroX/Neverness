using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using VisionGal.Managed.Assets;
using VisionGal.Managed.Core;
using VisionGal.Managed.Engine;
using VisionGal.Managed.Engine.Runtime;
using VisionGal.Managed.Object;

namespace VisionGal.Managed.Runtime;

/// <summary>
/// Phase 1–6：供 Native 透過 <c>load_assembly_and_get_function_pointer</c> 解析之 <c>[UnmanagedCallersOnly]</c> 匯出入口；
/// 執行期程式集透過專案參考納入 <b>VisionGal.Managed.Gameplay</b>，使 publish 目錄含 Phase 6 首包 DLL。
/// </summary>
public static class Entry
{
	/// <summary>Phase 1：<see cref="Smoke"/> 已被呼叫。</summary>
	public static volatile bool SmokeCalled;

	/// <summary>Phase 2：<see cref="BootstrapNativeApi"/> 已成功安裝 <c>VGNativeAPI</c> 並寫入 Log。</summary>
	public static volatile bool BootstrapNativeApiCompleted;

	/// <summary>Phase 3：引擎服務表已安裝且 <see cref="EngineNativeApiBootstrap.ExerciseStubInteropPath"/> 已執行。</summary>
	public static volatile bool BootstrapEngineInteropCompleted;

	/// <summary>Phase 5：Foundation 演練（Object / Scene / Assets）已完成。</summary>
	public static volatile bool BootstrapEngineFoundationCompleted;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Smoke 已完成。</summary>
	public const int FlagSmoke = 1 << 0;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Native API Bootstrap 已完成。</summary>
	public const int FlagNativeApi = 1 << 1;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Engine Interop 演練已完成。</summary>
	public const int FlagEngineInterop = 1 << 2;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Engine Foundation 演練已完成。</summary>
	public const int FlagEngineFoundation = 1 << 3;

	/// <summary>Phase 1：無參數 smoke，驗證 UCO 呼叫鏈路。</summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void Smoke()
	{
		SmokeCalled = true;
	}

	/// <summary>
	/// Phase 2：接收 Native 傳入之 <c>const VGNativeAPI*</c>（以 <see cref="nint"/> 傳遞），安裝函數表並經 ABI 觸發一次 <c>LogInfo</c>。
	/// </summary>
	/// <param name="nativeApiTable">Native 側 <c>VGNativeApi_GetDefaultTable()</c> 回傳之指標。</param>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void BootstrapNativeApi(nint nativeApiTable)
	{
		NativeApiBootstrap.Install(nativeApiTable);
		NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapNativeApi -> Native LogInfo (Phase2 ABI)"u8);
		BootstrapNativeApiCompleted = NativeApiBootstrap.IsInstalled;

		EngineNativeApiBootstrap.InstallFromNativeApiTable(nativeApiTable);
		EngineNativeApiBootstrap.ExerciseStubInteropPath();
		_ = EngineTime.FrameIndex;
		BootstrapEngineInteropCompleted = EngineNativeApiBootstrap.IsInstalled;
	}

	/// <summary>
	/// Phase 5：演練 <see cref="ObjectRegistry"/>、場景 JSON 往返與資產匯入；可選經 Native Object API 查詢存活狀態。
	/// </summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void BootstrapEngineFoundation()
	{
		// 清空註冊表與資產快取，避免與先前測試或同行程式殘留狀態交錯。
		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();

		// 經 LifetimeSystem 建立 SceneEntity 並註冊；若 Engine ABI 已安裝則稍後以 IsAlive 驗證 Native 控制代碼。
		var entity = LifetimeSystem.CreateAndRegister<VisionGal.Managed.Scene.SceneEntity>("SceneEntity");
		entity.DisplayName = "BootstrapEntity";

		var nativeObjectOk = !EngineNativeApiBootstrap.IsInstalled || NativeHandleBridge.IsAlive(entity.Handle);

		var scene = new VisionGal.Managed.Scene.Scene("BootstrapScene");
		scene.AddEntity(entity);

		// 場景 JSON 往返：僅驗證 DTO 與託管屬性一致，不涉及再水合。
		var json = scene.ToJson();
		var sceneRoundTripOk = scene.ValidateRoundTripDocument(json, entity.DisplayName);

		var expectedDisplayName = entity.DisplayName;
		// 模擬「載入新場景」：清空後自 JSON 再水合，應得到新 Id/Handle 但屬性值相同。
		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();

		var rehydratedScene = VisionGal.Managed.Scene.SceneRehydrator.RestoreFromJsonWithEntities(json);
		var rehydratedEntity = rehydratedScene?.Entities.Count == 1 ? rehydratedScene.Entities[0] : null;
		var sceneRehydrationOk =
			rehydratedScene != null &&
			rehydratedScene.Name == "BootstrapScene" &&
			rehydratedEntity != null &&
			rehydratedEntity.DisplayName == expectedDisplayName &&
			ObjectRegistry.Count >= 1 &&
			ObjectRegistry.TryGet(rehydratedEntity.Id, out _) &&
			(!EngineNativeApiBootstrap.IsInstalled || NativeHandleBridge.IsAlive(rehydratedEntity.Handle));

		// 資產匯入：虛擬路徑 → 決定性 GUID，並可自 AssetDatabase 反查。
		const string assetPath = "/assets/bootstrap/test.png";
		var imported = ImportPipeline.Import(assetPath);
		var assetOk = !imported.IsZero && AssetDatabase.TryResolveGuid(assetPath, out var resolved) && resolved == imported;

		BootstrapEngineFoundationCompleted =
			nativeObjectOk &&
			sceneRoundTripOk &&
			sceneRehydrationOk &&
			assetOk;

		NativeApiBootstrap.LogInfoUtf8(
			BootstrapEngineFoundationCompleted
				? "VisionGal.Managed.Runtime BootstrapEngineFoundation OK (Phase5)"u8
				: "VisionGal.Managed.Runtime BootstrapEngineFoundation FAILED"u8);
	}

	/// <summary>
	/// 供 GTest 讀取 Bootstrap 旗標（位元遮罩，見 <see cref="FlagSmoke"/> 等常數）；不依賴 stderr 日誌解析。
	/// </summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static int GetBootstrapFlags()
	{
		// 位元累加：供 Native GTest 以整數斷言各 Bootstrap 階段是否成功，無需解析日誌字串。
		var flags = 0;
		if (SmokeCalled)
		{
			flags |= FlagSmoke;
		}

		if (BootstrapNativeApiCompleted)
		{
			flags |= FlagNativeApi;
		}

		if (BootstrapEngineInteropCompleted)
		{
			flags |= FlagEngineInterop;
		}

		if (BootstrapEngineFoundationCompleted)
		{
			flags |= FlagEngineFoundation;
		}

		return flags;
	}
}
