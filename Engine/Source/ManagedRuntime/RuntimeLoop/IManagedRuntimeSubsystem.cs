namespace VisionGal.Managed.RuntimeLoop;

/// <summary>
/// 可注册到 <see cref="ManagedRuntimeScheduler"/> 的托管子系统接口，与 Native <c>IRuntimeSubsystem</c> 对称（纯 C#、无 P/Invoke 依赖）。
/// </summary>
public interface IManagedRuntimeSubsystem
{
	/// <summary>宿主启动阶段调用一次。</summary>
	void Initialize();

	/// <summary>宿主关闭阶段调用；应释放本帧外资源。</summary>
	void Shutdown();

	/// <summary>在对应 <see cref="TickGroup"/> 阶段每帧调用。</summary>
	void Tick(in ManagedRuntimeFrameContext context);

	/// <summary>本子系统所属 Tick 分组。</summary>
	RuntimeTickGroup TickGroup { get; }
}
