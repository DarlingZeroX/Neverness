namespace Neverness.Runtime.Engine;

/// <summary>
/// 与 Native NN_NATIVE_ENGINE_API_LAYOUT_VERSION（EngineAPIRegistry.h）数值一致；托管读表前必须相等，否则 EngineNativeApiBootstrap.InstallFromNativeApiTable 拒绝缓存。
/// </summary>
/// <remarks>
/// layout v28：新增 NNDiligentAPI 子表。
/// </remarks>
public static class NNNativeEngineApiConstants
{
	/// <summary>
	/// 当前 NNNativeEngineAPI 聚合体布局版本；破坏性子表字段变更时递增。当前为 28。
	/// v28：新增 NNDiligentAPI（4 个函数指针）子表。
	/// </summary>
	public const uint LayoutVersion = 28;
}
