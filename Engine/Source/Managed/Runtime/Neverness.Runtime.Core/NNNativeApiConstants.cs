namespace Neverness.Managed.Core;

/// <summary>
/// 與 Native 側 C 標頭 <c>NativeAPI.h</c> 中 <c>NN_NATIVE_API_VERSION</c> 保持數值一致。
/// </summary>
public static class NNNativeApiConstants
{
	/// <summary>当前 <c>NNNativeAPI</c> 结构体布局版本；破坏性变更时递增。</summary>
	public const uint ApiVersion = 2;
}
