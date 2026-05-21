using Neverness.Runtime.RuntimeLoop;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// <see cref="RuntimeLoop"/> 与 Native RuntimeScheduler 管线顺序对称。
/// </summary>
public sealed class RuntimeLoopTests
{
	private sealed class PhaseLogSubsystem : IManagedRuntimeSubsystem
	{
		private readonly RuntimeTickGroup _group;
		private readonly List<string> _log;

		public PhaseLogSubsystem(RuntimeTickGroup group, List<string> log)
		{
			_group = group;
			_log = log;
		}

		public RuntimeTickGroup TickGroup => _group;

		public void Initialize()
		{
		}

		public void Shutdown()
		{
		}

		public void Tick(in ManagedRuntimeFrameContext context)
		{
			_ = context;
			_log.Add(_group.ToString());
		}
	}

	private sealed class FixedCounterSubsystem : IManagedRuntimeSubsystem
	{
		public int FixedTickCount;

		public RuntimeTickGroup TickGroup => RuntimeTickGroup.FixedUpdate;

		public void Initialize()
		{
		}

		public void Shutdown()
		{
		}

		public void Tick(in ManagedRuntimeFrameContext context)
		{
			_ = context;
			FixedTickCount++;
		}
	}

	[Fact]
	public void Tick_invokes_early_update_before_update()
	{
		var log = new List<string>();
		var loop = new RuntimeLoop.RuntimeLoop();
		loop.Register(new PhaseLogSubsystem(RuntimeTickGroup.Update, log));
		loop.Register(new PhaseLogSubsystem(RuntimeTickGroup.EarlyUpdate, log));
		loop.InitializeRegistered();
		var ctx = new ManagedRuntimeFrameContext(
			deltaTimeSeconds: 1f / 60f,
			totalTimeSeconds: 1f / 60f,
			frameIndex: 1,
			fixedDeltaTimeSeconds: FrameScheduler.DefaultFixedDeltaTimeSeconds);
		loop.Tick(in ctx);
		Assert.Equal(new[] { "EarlyUpdate", "Update" }, log);
		loop.ShutdownRegistered();
	}

	[Fact]
	public void Fixed_update_runs_at_most_max_steps_per_frame()
	{
		var loop = new RuntimeLoop.RuntimeLoop();
		loop.Frames.FixedDeltaTimeSeconds = 0.02f;
		var fixedCounter = new FixedCounterSubsystem();
		loop.Register(fixedCounter);
		loop.InitializeRegistered();
		var ctx = new ManagedRuntimeFrameContext(
			deltaTimeSeconds: 0.11f,
			totalTimeSeconds: 0.11f,
			frameIndex: 1,
			fixedDeltaTimeSeconds: 0.02f);
		loop.Tick(in ctx);
		Assert.Equal(FrameScheduler.MaxFixedStepsPerFrame, fixedCounter.FixedTickCount);
		loop.ShutdownRegistered();
	}
}
