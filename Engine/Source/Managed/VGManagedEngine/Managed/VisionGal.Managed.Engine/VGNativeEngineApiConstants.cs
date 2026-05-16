namespace VisionGal.Managed.Engine;

/// <summary>
/// 與 Native <c>VG_NATIVE_ENGINE_API_LAYOUT_VERSION</c>（<c>EngineAPIRegistry.h</c>）數值一致；託管讀表前必須相等，否則 <see cref="EngineNativeApiBootstrap.InstallFromNativeApiTable"/> 拒絕快取。
/// </summary>
/// <remarks>
/// **layout v5**：<c>VGEntityAPI</c> 子表尾部含 <c>getRuntimeTick</c>（見 MANAGED 總覽 **§2.7.1**）。若僅升 Native 未同步本常數，託管層將無法安裝引擎服務表。
/// </remarks>
public static class VGNativeEngineApiConstants
{
	/// <summary>
	/// 當前 <c>VGNativeEngineAPI</c> 聚合體佈局版本；破壞性子表欄位變更時遞增。當前為 **5**（含 <c>VGEntityApi.GetRuntimeTick</c> 鏡像欄位）。
	/// </summary>
	public const uint LayoutVersion = 5;

	/// <summary>
	/// 與 Native <c>VG_ENTITY_SERVICE_ABI_TOKEN</c>（<c>EntityAPI.h</c>）一致之服務子表冒煙魔數（ASCII「VGEn」小端）；僅用於驗證 <c>VGEntityAPI</c> 已接線，不代表託管 <c>EntityWorld</c> 已與 Native 打通。
	/// </summary>
	public const uint EntityServiceAbiToken = 0x5647456Eu;
}
