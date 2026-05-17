using Neverness.Managed.RuntimeLoop;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>
/// P0-1：<see cref="ManagedRuntimeScheduler"/> 與 Native <c>RuntimeScheduler</c> 管線順序對稱（Early → Fixed → Update → Late → Render）。
/// </summary>
public sealed class ManagedRuntimeSchedulerTests
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
		var scheduler = new ManagedRuntimeScheduler();
		// 故意先註冊 Update 再 Early，驗證「階段順序」而非註冊順序。
		scheduler.Register(new PhaseLogSubsystem(RuntimeTickGroup.Update, log));
		scheduler.Register(new PhaseLogSubsystem(RuntimeTickGroup.EarlyUpdate, log));
		scheduler.InitializeRegistered();
		var ctx = new ManagedRuntimeFrameContext(
			deltaTimeSeconds: 1f / 60f,
			totalTimeSeconds: 1f / 60f,
			frameIndex: 1,
			fixedDeltaTimeSeconds: scheduler.FixedDeltaTimeSeconds);
		scheduler.Tick(in ctx);
		Assert.Equal(new[] { "EarlyUpdate", "Update" }, log);
		scheduler.ShutdownRegistered();
	}

	[Fact]
	public void Fixed_update_runs_at_most_max_steps_per_frame()
	{
		var scheduler = new ManagedRuntimeScheduler { FixedDeltaTimeSeconds = 0.02f };
		var fixedCounter = new FixedCounterSubsystem();
		scheduler.Register(fixedCounter);
		scheduler.InitializeRegistered();
		// 0.11s / 0.02s = 5.5 → 受上限約束應為 5 次 Fixed 回調（與 Native 一致）。
		var ctx = new ManagedRuntimeFrameContext(
			deltaTimeSeconds: 0.11f,
			totalTimeSeconds: 0.11f,
			frameIndex: 1,
			fixedDeltaTimeSeconds: 0.02f);
		scheduler.Tick(in ctx);
		Assert.Equal(ManagedRuntimeScheduler.MaxFixedStepsPerFrame, fixedCounter.FixedTickCount);
		scheduler.ShutdownRegistered();
	}
}
