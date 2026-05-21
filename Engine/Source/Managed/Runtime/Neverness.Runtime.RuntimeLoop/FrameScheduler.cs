// Neverness.Runtime.RuntimeLoop — FixedUpdate 累加与每帧最大步数限制。

namespace Neverness.Runtime.RuntimeLoop;

/// <summary>
/// 固定时间步累加器；与 Native <c>RuntimeScheduler::RunFixedPasses</c> 语义对齐。
/// </summary>
public sealed class FrameScheduler
{
	public const float DefaultFixedDeltaTimeSeconds = 1f / 50f;
	public const int MaxFixedStepsPerFrame = 5;

	private float _fixedAccumulator;
	private float _fixedDeltaTimeSeconds = DefaultFixedDeltaTimeSeconds;

	/// <summary>固定步长（秒）。</summary>
	public float FixedDeltaTimeSeconds
	{
		get => _fixedDeltaTimeSeconds;
		set => _fixedDeltaTimeSeconds = value;
	}

	/// <summary>重置累加器（Shutdown 时调用）。</summary>
	public void ResetAccumulator() => _fixedAccumulator = 0f;

	/// <summary>
	/// 根据本帧 delta 执行 0..<see cref="MaxFixedStepsPerFrame"/> 次 Fixed 回调。
	/// </summary>
	public void RunFixedPasses(
		float deltaTimeSeconds,
		in ManagedRuntimeFrameContext baseContext,
		Action<ManagedRuntimeFrameContext> onFixedTick)
	{
		ArgumentNullException.ThrowIfNull(onFixedTick);
		_fixedAccumulator += deltaTimeSeconds;
		var step = _fixedDeltaTimeSeconds;
		if (step <= 0f)
		{
			return;
		}

		var iterations = 0;
		while (_fixedAccumulator >= step && iterations < MaxFixedStepsPerFrame)
		{
			_fixedAccumulator -= step;
			iterations++;
			var fixedCtx = new ManagedRuntimeFrameContext(
				step,
				baseContext.TotalTimeSeconds,
				baseContext.FrameIndex,
				step);
			onFixedTick(fixedCtx);
		}
	}
}
