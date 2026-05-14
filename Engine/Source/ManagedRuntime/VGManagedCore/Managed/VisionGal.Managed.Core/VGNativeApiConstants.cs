namespace VisionGal.Managed.Core;

/// <summary>
/// 与 Native 侧 C 头文件 <c>NativeAPI.h</c> 中 <c>VG_NATIVE_API_VERSION</c> 保持数值一致。
/// </summary>
public static class VGNativeApiConstants
{
	/// <summary>当前 <c>VGNativeAPI</c> 结构体布局版本；破坏性变更时递增。</summary>
	public const uint ApiVersion = 1;
}
