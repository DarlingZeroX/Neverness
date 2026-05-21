namespace Neverness.Runtime.Engine;

/// <summary>
/// 與 Native <c>NNWindowHandle</c>（<c>WindowTypes.h</c>）對齊之不透明窗口句柄。
/// </summary>
public readonly struct NNWindowHandle
{
	/// <summary>無效句柄（Native <c>NN_INVALID_WINDOW_HANDLE</c>）。</summary>
	public static readonly NNWindowHandle Invalid = new(0);

	public ulong Value { get; }

	public NNWindowHandle(ulong value) => Value = value;

	public bool IsValid => Value != 0;

	public static implicit operator ulong(NNWindowHandle handle) => handle.Value;
}
