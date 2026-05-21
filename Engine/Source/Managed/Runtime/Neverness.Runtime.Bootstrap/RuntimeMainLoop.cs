// Neverness.Runtime.Bootstrap — 单帧推进门面；对齐 Native NNEngineRuntimeHost_Tick 之后的托管 Tick。

using Neverness.Runtime.RuntimeLoop;

namespace Neverness.Runtime.Bootstrap;

/// <summary>
/// 托管主循环单帧推进；由 Native <c>Entry.RuntimeTick</c> 或 <see cref="RuntimeBootstrap"/> 外循环调用。
/// </summary>
public static class RuntimeMainLoop
{
	private static float s_totalTimeSeconds;
	private static ulong s_frameIndex;

	/// <summary>当前累计帧索引（托管侧计数，与 EngineTime 可并存）。</summary>
	public static ulong FrameIndex => s_frameIndex;

	/// <summary>请求退出托管外循环（仅 <see cref="NativeBootstrapRunMode.ManagedOuterLoop"/> 有效）。</summary>
	public static void RequestQuit() => RuntimeBootstrap.RequestQuit();

	/// <summary>
	/// 推进一帧托管 Kernel：Early → Fixed → Update → Late → FlushMainThread → Render。
	/// </summary>
	/// <param name="deltaTimeSeconds">本帧增量时间（秒），通常来自 Native Timing。</param>
	public static void Tick(float deltaTimeSeconds)
	{
		if (!RuntimeInitializer.IsInitialized || RuntimeInitializer.Loop == null)
		{
			return;
		}

		s_totalTimeSeconds += deltaTimeSeconds;
		var ctx = new ManagedRuntimeFrameContext(
			deltaTimeSeconds,
			s_totalTimeSeconds,
			s_frameIndex,
			deltaTimeSeconds);
		RuntimeInitializer.Loop.Tick(in ctx);
		s_frameIndex++;
	}
}
