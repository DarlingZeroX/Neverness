using Neverness.Managed.Interop;

namespace Neverness.Managed.Engine.Runtime;

/// <summary>
/// 透過 <see cref="EngineNativeApiBootstrap"/> 讀取之 **時間服務** 薄封裝；未安裝 ABI 時回傳零值。
/// </summary>
public static unsafe class EngineTime
{
	/// <summary>上一 <c>Tick</c> 之增量時間（秒）。</summary>
	public static float DeltaTime =>
		EngineNativeApiBootstrap.IsInstalled && EngineNativeApiBootstrap.EngineApi.Timing.GetDeltaTime != null
			? EngineNativeApiBootstrap.EngineApi.Timing.GetDeltaTime()
			: 0f;

	/// <summary>自 Runtime 啟動以來累積時間（秒）。</summary>
	public static float TotalTime =>
		EngineNativeApiBootstrap.IsInstalled && EngineNativeApiBootstrap.EngineApi.Timing.GetTotalTime != null
			? EngineNativeApiBootstrap.EngineApi.Timing.GetTotalTime()
			: 0f;

	/// <summary>已執行之 <c>Tick</c> 次數（由 Native Runtime 定義）。</summary>
	public static ulong FrameIndex =>
		EngineNativeApiBootstrap.IsInstalled && EngineNativeApiBootstrap.EngineApi.Timing.GetFrameIndex != null
			? EngineNativeApiBootstrap.EngineApi.Timing.GetFrameIndex()
			: 0UL;
}
