using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using VisionGal.Managed.Assets;
using VisionGal.Managed.Core;
using VisionGal.Managed.Engine;
using VisionGal.Managed.Engine.Runtime;
using VisionGal.Managed.Gameplay;
using VisionGal.Managed.Object;
using VisionGal.Managed.Scene;

namespace VisionGal.Managed.Runtime;

/// <summary>
/// Phase 1–6：供 Native 透過 <c>load_assembly_and_get_function_pointer</c> 解析之 <c>[UnmanagedCallersOnly]</c> 匯出入口；
/// 執行期程式集透過專案參考納入 <b>VisionGal.Managed.Gameplay</b>（含 <see cref="BootstrapGameplay"/> 之 Phase 6 演練）。
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

	/// <summary>Phase 6：<see cref="BootstrapGameplay"/>（變數表 JSON、序列含場景再水合、會話快照往返、對白 Stub）已完成。</summary>
	public static volatile bool BootstrapGameplayCompleted;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Smoke 已完成。</summary>
	public const int FlagSmoke = 1 << 0;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Native API Bootstrap 已完成。</summary>
	public const int FlagNativeApi = 1 << 1;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Engine Interop 演練已完成。</summary>
	public const int FlagEngineInterop = 1 << 2;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Engine Foundation 演練已完成。</summary>
	public const int FlagEngineFoundation = 1 << 3;

	/// <summary><see cref="GetBootstrapFlags"/> 位元遮罩：Gameplay 演練（Phase 6 slice）已完成。</summary>
	public const int FlagGameplay = 1 << 4;

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
	/// Phase 6：演練 <see cref="GameplayVariableStore"/> JSON 往返、<see cref="SequenceRunner"/>（含場景 JSON → <see cref="SceneRehydrator"/> 再水合與變數同步）、<see cref="GameplaySessionSnapshot"/> 根層往返、<see cref="DialoguePresenter"/>；須先呼叫 <see cref="BootstrapNativeApi"/>。
	/// </summary>
	[UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvStdcall) })]
	public static void BootstrapGameplay()
	{
		BootstrapGameplayCompleted = false;

		var store = new GameplayVariableStore();
		store.Set("gpBootstrap", false);

		var json = store.ToJson();
		if (!GameplayVariableStore.TryParseFromJson(json, out var roundTrip) || roundTrip is null)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (json round-trip)"u8);
			return;
		}

		if (!roundTrip.TryGet("gpBootstrap", out var v0) || v0 is not bool b0 || b0)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (json value)"u8);
			return;
		}

		// 與 Foundation 演練隔離：清空註冊表後建立「待載入」場景快照，再清空並以序列步驟再水合，模擬劇本邊載入邊寫變數。
		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();

		const string expectedRehydratedTitle = "GameplayBootstrapEntity";
		var preloadEntity = LifetimeSystem.CreateAndRegister<SceneEntity>("SceneEntity");
		preloadEntity.DisplayName = expectedRehydratedTitle;
		var preloadScene = new VisionGal.Managed.Scene.Scene("GameplayBootstrapScene");
		preloadScene.AddEntity(preloadEntity);
		var sceneJson = preloadScene.ToJson();

		ObjectRegistry.ClearForTesting();
		AssetDatabase.ClearForTesting();

		// 使用往返後之變數表承接後續序列，避免與上方暫存狀態混淆。
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new RehydrateSceneSequenceStep(sceneJson),
				new SyncFirstEntityDisplayNameToVariableSequenceStep("rehydratedTitle"),
				new SetVariableSequenceStep("gpBootstrap", true),
				new PresentDialogueSequenceStep(0, "VisionGal.Managed.Runtime BootstrapGameplay (Phase6)"),
			});

		if (!runner.Run(roundTrip))
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (sequence)"u8);
			return;
		}

		if (!roundTrip.TryGet("gpBootstrap", out var v1) || v1 is not bool b1 || !b1)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (variable)"u8);
			return;
		}

		if (!roundTrip.TryGet("rehydratedTitle", out var titleObj) || titleObj is not string title || title != expectedRehydratedTitle)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (scene sync)"u8);
			return;
		}

		// Phase 6 slice 4：會話快照（變數表子 JSON + 場景 JSON）根層往返，驗證與 VersionTolerance 一致之載入路徑。
		var sessionSnapshot = GameplaySessionSnapshot.Capture(roundTrip, sceneJson);
		var sessionJson = sessionSnapshot.ToJson();
		if (!GameplaySessionSnapshot.TryParseFromJson(sessionJson, out var restoredSession) || restoredSession is null)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (session snapshot parse)"u8);
			return;
		}

		if (restoredSession.SceneJson != sceneJson)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (session sceneJson)"u8);
			return;
		}

		var applied = new GameplayVariableStore();
		if (!restoredSession.ApplyTo(applied))
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (session ApplyTo)"u8);
			return;
		}

		if (!applied.TryGet("gpBootstrap", out var v2) || v2 is not bool b2 || !b2 ||
		    !applied.TryGet("rehydratedTitle", out var t2) || t2 is not string s2 || s2 != expectedRehydratedTitle)
		{
			NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay FAILED (session variables)"u8);
			return;
		}

		BootstrapGameplayCompleted = true;
		NativeApiBootstrap.LogInfoUtf8("VisionGal.Managed.Runtime BootstrapGameplay OK (Phase6)"u8);
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

		if (BootstrapGameplayCompleted)
		{
			flags |= FlagGameplay;
		}

		return flags;
	}
}
