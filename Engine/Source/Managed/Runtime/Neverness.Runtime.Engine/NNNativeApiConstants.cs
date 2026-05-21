namespace Neverness.Runtime.Core;

/// <summary>
/// 与 Native 侧 C 头 <c>NativeAPI.h</c> 中 <c>NN_NATIVE_API_VERSION</c> 保持数值一致。
/// </summary>
public static class NNNativeApiConstants
{
	/// <summary>当前 <c>NNNativeAPI</c> 结构体布局版本；破坏性变更时递增。</summary>
	public const uint ApiVersion = 2;
}
