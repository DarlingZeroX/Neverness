using Neverness.Managed.RuntimeLoop;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>
/// <see cref="MainThreadDispatcher"/> 跨线程 Post 与主线程 Drain 顺序。
/// </summary>
public sealed class MainThreadDispatcherTests
{
	[Fact]
	public void Drain_executes_posted_actions_in_order()
	{
		var dispatcher = new MainThreadDispatcher();
		var log = new List<int>();
		dispatcher.Post(() => log.Add(1));
		dispatcher.Post(() => log.Add(2));
		dispatcher.Drain();
		Assert.Equal(new[] { 1, 2 }, log);
	}

	[Fact]
	public void Post_from_background_thread_drains_on_main()
	{
		var dispatcher = new MainThreadDispatcher();
		var log = new List<int>();
		using var gate = new ManualResetEventSlim(false);
		ThreadPool.QueueUserWorkItem(_ =>
		{
			dispatcher.Post(() => log.Add(42));
			gate.Set();
		});
		gate.Wait(TimeSpan.FromSeconds(5));
		dispatcher.Drain();
		Assert.Equal(new[] { 42 }, log);
	}
}
