using System;

namespace Neverness.Runtime.RuntimeLoop;

/// <summary>
/// <b>已弃用</b>：请使用 <see cref="RuntimeLoop"/>。本类为兼容测试保留的薄包装。
/// </summary>
[Obsolete("请改用 RuntimeLoop；本包装将在下一发行周期移除。")]
public sealed class ManagedRuntimeScheduler
{
	public const float DefaultFixedDeltaTimeSeconds = FrameScheduler.DefaultFixedDeltaTimeSeconds;
	public const int MaxFixedStepsPerFrame = FrameScheduler.MaxFixedStepsPerFrame;

	private readonly RuntimeLoop _loop = new();

	/// <summary>固定步长（秒）。</summary>
	public float FixedDeltaTimeSeconds
	{
		get => _loop.Frames.FixedDeltaTimeSeconds;
		set => _loop.Frames.FixedDeltaTimeSeconds = value;
	}

	/// <summary>注册子系统。</summary>
	public void Register(IManagedRuntimeSubsystem subsystem) => _loop.Register(subsystem);

	/// <summary>取消注册。</summary>
	public bool Unregister(IManagedRuntimeSubsystem subsystem) => _loop.Unregister(subsystem);

	/// <summary>初始化已注册子系统。</summary>
	public void InitializeRegistered() => _loop.InitializeRegistered();

	/// <summary>关闭子系统。</summary>
	public void ShutdownRegistered() => _loop.ShutdownRegistered();

	/// <summary>推进一帧。</summary>
	public void Tick(in ManagedRuntimeFrameContext context) => _loop.Tick(in context);
}
