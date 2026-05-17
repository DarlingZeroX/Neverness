namespace VisionGal.Managed.RuntimeLoop;

/// <summary>
/// 单帧只读上下文，对应 Native <c>RuntimeFrameContext</c>；由宿主在每帧调用 <see cref="ManagedRuntimeScheduler.Tick"/> 前构造。
/// </summary>
public readonly struct ManagedRuntimeFrameContext
{
	public ManagedRuntimeFrameContext(
		float deltaTimeSeconds,
		float totalTimeSeconds,
		ulong frameIndex,
		float fixedDeltaTimeSeconds)
	{
		DeltaTimeSeconds = deltaTimeSeconds;
		TotalTimeSeconds = totalTimeSeconds;
		FrameIndex = frameIndex;
		FixedDeltaTimeSeconds = fixedDeltaTimeSeconds;
	}

	/// <summary>上一帧到本帧的间隔（秒）。</summary>
	public float DeltaTimeSeconds { get; }

	/// <summary>自启动以来的累计时间（秒）。</summary>
	public float TotalTimeSeconds { get; }

	/// <summary>单调帧序号（通常从 1 起，依宿主约定）。</summary>
	public ulong FrameIndex { get; }

	/// <summary>固定步长（秒），用于 <see cref="RuntimeTickGroup.FixedUpdate"/> 子系统与调度器累加器。</summary>
	public float FixedDeltaTimeSeconds { get; }
}
