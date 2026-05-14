namespace VisionGal.Managed.Engine;

/// <summary>
/// 與 Native <c>VG_NATIVE_ENGINE_API_LAYOUT_VERSION</c>（<c>EngineAPIRegistry.h</c>）數值一致。
/// </summary>
public static class VGNativeEngineApiConstants
{
	/// <summary>當前 <c>VGNativeEngineAPI</c> 聚合體佈局版本；破壞性子表欄位變更時遞增。</summary>
	public const uint LayoutVersion = 1;
}
