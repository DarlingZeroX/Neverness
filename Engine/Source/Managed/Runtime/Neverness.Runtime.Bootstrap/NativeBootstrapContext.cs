// Neverness.Runtime.Bootstrap — 托管 Runtime 启动上下文（与 Native 外循环 / 纯托管 Exe 双模式对齐）。

namespace Neverness.Managed.Bootstrap;

/// <summary>
/// 描述托管 Runtime 的启动参数；由 Native <c>Entry.Bootstrap</c> 或 <see cref="RuntimeBootstrap.Start"/> 传入。
/// </summary>
public readonly struct NativeBootstrapContext
{
	/// <summary>Native <c>NNNativeApi_GetDefaultTable()</c> 回传指针（<c>const NNNativeAPI*</c>）。</summary>
	public nint NativeApiTable { get; init; }

	/// <summary>可选 runtimeconfig.json 路径；Legacy Host 或产品宿主用于 CoreCLR 配置。</summary>
	public string? OptionalConfigPath { get; init; }

	/// <summary>运行模式：Native 外循环每帧回调 <c>RuntimeTick</c>，或托管自持外循环（调试用）。</summary>
	public NativeBootstrapRunMode RunMode { get; init; }
}

/// <summary>
/// <see cref="NativeBootstrapContext.RunMode"/>：决定 <see cref="RuntimeBootstrap.Start"/> 是否阻塞进入主循环。
/// </summary>
public enum NativeBootstrapRunMode
{
	/// <summary>默认：仅初始化；帧推进由 Native 调用 <c>Entry.RuntimeTick</c>。</summary>
	NativeDriven,

	/// <summary>调试：Start 内进入托管外循环（如 Neverness.Runtime.App headless）。</summary>
	ManagedOuterLoop,
}
