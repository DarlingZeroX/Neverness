// Neverness.Runtime.RuntimeLoop — 主线程委托队列；对齐 Native RuntimeScheduler::FlushMainThreadDelegates。

namespace Neverness.Runtime.RuntimeLoop;

/// <summary>
/// 线程安全的主线程工作队列；在 <see cref="RuntimeLoop.Tick"/> 的 LateUpdate 之后、Render 之前排空。
/// </summary>
public sealed class MainThreadDispatcher
{
	private readonly object _gate = new();
	private readonly Queue<Action> _pending = new();

	/// <summary>将工作项投递到主线程队列（可从任意线程调用）。</summary>
	public void Post(Action work)
	{
		ArgumentNullException.ThrowIfNull(work);
		lock (_gate)
		{
			_pending.Enqueue(work);
		}
	}

	/// <summary>在主线程排空队列（由 <see cref="RuntimeLoop"/> 每帧调用一次）。</summary>
	public void Drain()
	{
		while (true)
		{
			Action? action;
			lock (_gate)
			{
				if (_pending.Count == 0)
				{
					return;
				}

				action = _pending.Dequeue();
			}

			action();
		}
	}

	/// <summary>清空未执行项（Shutdown 时可选调用）。</summary>
	public void Clear()
	{
		lock (_gate)
		{
			_pending.Clear();
		}
	}
}
