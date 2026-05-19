// Neverness.Runtime.RuntimeLoop — 子系统分桶注册与按 TickGroup 调度。

namespace Neverness.Managed.RuntimeLoop;

/// <summary>
/// 管理 <see cref="IManagedRuntimeSubsystem"/> 注册与按 <see cref="RuntimeTickGroup"/> 分桶 Tick；
/// 与 Native <c>RuntimeSubsystemCollection</c> 对称。
/// </summary>
public sealed class SubsystemScheduler
{
	private readonly List<IManagedRuntimeSubsystem>[] _buckets = CreateEmptyBuckets();

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

	/// <summary>注册子系统；重复实例忽略。</summary>
	public void Register(IManagedRuntimeSubsystem subsystem)
	{
		ArgumentNullException.ThrowIfNull(subsystem);
		var list = _buckets[BucketIndex(subsystem.TickGroup)];
		if (!list.Contains(subsystem))
		{
			list.Add(subsystem);
		}
	}

	/// <summary>取消注册。</summary>
	public bool Unregister(IManagedRuntimeSubsystem subsystem)
	{
		ArgumentNullException.ThrowIfNull(subsystem);
		return _buckets[BucketIndex(subsystem.TickGroup)].Remove(subsystem);
	}

	/// <summary>按分组与注册顺序调用 Initialize。</summary>
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

	/// <summary>逆序 Shutdown 并清空各桶。</summary>
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
	}

	/// <summary>对指定 Tick 分组调用 Tick。</summary>
	public void TickBucket(RuntimeTickGroup group, in ManagedRuntimeFrameContext context)
	{
		foreach (var s in _buckets[BucketIndex(group)])
		{
			s.Tick(in context);
		}
	}
}
