namespace Neverness.Runtime.Engine.Runtime;

/// <summary>
/// 通过 <see cref="EngineNativeApiCache"/> 读取的**时间服务**薄封装；未安装 ABI 时返回零值。
/// </summary>
public static unsafe class EngineTime
{
	/// <summary>上一 <c>Tick</c> 的增量时间（秒）。</summary>
	public static float DeltaTime => 0f;

	/// <summary>自 Runtime 启动以来累积时间（秒）。</summary>
	public static float TotalTime => 0f;

	/// <summary>已执行的 <c>Tick</c> 次数（由 Native Runtime 定义）。</summary>
	public static ulong FrameIndex => 0UL;
}
