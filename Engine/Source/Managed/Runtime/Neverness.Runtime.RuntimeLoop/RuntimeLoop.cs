// Neverness.Runtime.RuntimeLoop — 托管 Kernel 主循环：组合帧调度、子系统调度与主线程队列。

namespace Neverness.Runtime.RuntimeLoop;

/// <summary>
/// 托管 Runtime 主循环；管线顺序与 Native <c>RuntimeScheduler</c> 一致：
/// EarlyUpdate → FixedUpdate(0..N) → Update → LateUpdate → FlushMainThreadDelegates → Render。
/// </summary>
public sealed class RuntimeLoop
{
	private readonly SubsystemScheduler _subsystems = new();
	private readonly FrameScheduler _frames = new();
	private readonly MainThreadDispatcher _mainThread = new();

	/// <summary>主线程委托队列（LateUpdate 之后 Drain）。</summary>
	public MainThreadDispatcher MainThread => _mainThread;

	/// <summary>子系统调度器。</summary>
	public SubsystemScheduler Subsystems => _subsystems;

	/// <summary>固定步长调度器。</summary>
	public FrameScheduler Frames => _frames;

	/// <summary>注册子系统。</summary>
	public void Register(IManagedRuntimeSubsystem subsystem) => _subsystems.Register(subsystem);

	/// <summary>取消注册。</summary>
	public bool Unregister(IManagedRuntimeSubsystem subsystem) => _subsystems.Unregister(subsystem);

	/// <summary>初始化已注册子系统。</summary>
	public void InitializeRegistered() => _subsystems.InitializeRegistered();

	/// <summary>关闭子系统并重置 Fixed 累加器。</summary>
	public void ShutdownRegistered()
	{
		_subsystems.ShutdownRegistered();
		_frames.ResetAccumulator();
		_mainThread.Clear();
	}

	/// <summary>推进一帧。</summary>
	public void Tick(in ManagedRuntimeFrameContext context)
	{
		_subsystems.TickBucket(RuntimeTickGroup.EarlyUpdate, in context);
		_frames.RunFixedPasses(
			context.DeltaTimeSeconds,
			in context,
			fixedCtx => _subsystems.TickBucket(RuntimeTickGroup.FixedUpdate, in fixedCtx));
		_subsystems.TickBucket(RuntimeTickGroup.Update, in context);
		_subsystems.TickBucket(RuntimeTickGroup.LateUpdate, in context);
		_mainThread.Drain();
		_subsystems.TickBucket(RuntimeTickGroup.Render, in context);
	}
}
