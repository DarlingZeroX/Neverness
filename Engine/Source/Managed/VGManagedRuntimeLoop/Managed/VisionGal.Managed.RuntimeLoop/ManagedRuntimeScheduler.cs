using System;
using System.Collections.Generic;

namespace VisionGal.Managed.RuntimeLoop;

/// <summary>
/// 托管侧 **Runtime Loop** 调度器（<b>P0-1</b>），与 Native <c>RuntimeScheduler</c> 管线顺序对称：Early → Fixed（0..N）→ Update → Late → Render。
/// 首包无引擎依赖，供 Gameplay / 测试宿主自行驱动。
/// </summary>
public sealed class ManagedRuntimeScheduler
{
	public const float DefaultFixedDeltaTimeSeconds = 1f / 50f;
	public const int MaxFixedStepsPerFrame = 5;

	private readonly List<IManagedRuntimeSubsystem>[] _buckets =
		CreateEmptyBuckets();

	private float _fixedAccumulator;
	private float _fixedDeltaTimeSeconds = DefaultFixedDeltaTimeSeconds;

	private static List<IManagedRuntimeSubsystem>[] CreateEmptyBuckets()
	{
		var n = Enum.GetValues<RuntimeTickGroup>().Length;
		var arr = new List<IManagedRuntimeSubsystem>[n];
		for (var i = 0; i < n; i++)
		{
			arr[i] = new List<IManagedRuntimeSubsystem>();
		}

		return arr;
	}

	private static int BucketIndex(RuntimeTickGroup g) => (int)g;

	/// <summary>固定步长（秒）；默认 1/50。</summary>
	public float FixedDeltaTimeSeconds
	{
		get => _fixedDeltaTimeSeconds;
		set => _fixedDeltaTimeSeconds = value;
	}

	/// <summary>注册子系统；重复注册同一实例会被忽略。</summary>
	public void Register(IManagedRuntimeSubsystem subsystem)
	{
		ArgumentNullException.ThrowIfNull(subsystem);
		var list = _buckets[BucketIndex(subsystem.TickGroup)];
		if (list.Contains(subsystem))
		{
			return;
		}

		list.Add(subsystem);
	}

	/// <summary>取消注册；返回是否曾存在。</summary>
	public bool Unregister(IManagedRuntimeSubsystem subsystem)
	{
		ArgumentNullException.ThrowIfNull(subsystem);
		return _buckets[BucketIndex(subsystem.TickGroup)].Remove(subsystem);
	}

	/// <summary>对已注册子系统调用 <see cref="IManagedRuntimeSubsystem.Initialize"/>（按分组与注册顺序）。</summary>
	public void InitializeRegistered()
	{
		foreach (var list in _buckets)
		{
			foreach (var s in list)
			{
				s.Initialize();
			}
		}
	}

	/// <summary>逆序调用 <see cref="IManagedRuntimeSubsystem.Shutdown"/>，并清空 Fixed 累加器。</summary>
	public void ShutdownRegistered()
	{
		for (var gi = _buckets.Length - 1; gi >= 0; gi--)
		{
			var list = _buckets[gi];
			for (var i = list.Count - 1; i >= 0; i--)
			{
				list[i].Shutdown();
			}
		}

		_fixedAccumulator = 0f;
	}

	/// <summary>推进一帧：先累加 Fixed，再按阶段调用各子系统 <see cref="IManagedRuntimeSubsystem.Tick"/>。</summary>
	public void Tick(in ManagedRuntimeFrameContext context)
	{
		_fixedAccumulator += context.DeltaTimeSeconds;

		TickBucket(RuntimeTickGroup.EarlyUpdate, in context);
		RunFixedPasses(in context);
		TickBucket(RuntimeTickGroup.Update, in context);
		TickBucket(RuntimeTickGroup.LateUpdate, in context);
		FlushMainThreadDelegates();
		TickBucket(RuntimeTickGroup.Render, in context);
	}

	private void TickBucket(RuntimeTickGroup group, in ManagedRuntimeFrameContext context)
	{
		foreach (var s in _buckets[BucketIndex(group)])
		{
			s.Tick(in context);
		}
	}

	private void RunFixedPasses(in ManagedRuntimeFrameContext context)
	{
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
				context.TotalTimeSeconds,
				context.FrameIndex,
				step);
			TickBucket(RuntimeTickGroup.FixedUpdate, in fixedCtx);
		}
	}

	/// <summary>占位：未来在此 drain 主线程委托队列（与 Native <c>FlushMainThreadDelegates</c> 对齐）。</summary>
	private static void FlushMainThreadDelegates()
	{
	}
}
