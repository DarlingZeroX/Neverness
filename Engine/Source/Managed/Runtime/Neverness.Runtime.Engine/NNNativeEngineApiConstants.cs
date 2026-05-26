namespace Neverness.Runtime.Engine;

/// <summary>
/// 與 Native <c>NN_NATIVE_ENGINE_API_LAYOUT_VERSION</c>（<c>EngineAPIRegistry.h</c>）數值一致；託管讀表前必須相等，否則 <see cref="Neverness.Runtime.Interop.EngineNativeApiBootstrap.InstallFromNativeApiTable"/> 拒絕快取。
/// </summary>
/// <remarks>
/// **layout v14**：新增 <c>NNEditorSceneAPI</c> 子表（Editor 快照查詢），獨立於 <c>NNSceneAPI</c>。
/// </remarks>
public static class NNNativeEngineApiConstants
{
	/// <summary>
	/// 當前 <c>NNNativeEngineAPI</c> 聚合體佈局版本；破壞性子表欄位變更時遞增。當前為 **20**。
	/// v20：新增 NNRenderAssetAPI 子表（GPU Texture 資源管理）。
	/// </summary>
	public const uint LayoutVersion = 20;

	/// <summary>
	/// 與 Native <c>NN_ENTITY_SERVICE_ABI_TOKEN</c>（<c>EntityAPI.h</c>）一致之服務子表冒煙魔數（ASCII「NNEn」小端）；僅用於驗證 <c>NNEntityAPI</c> 已接線，不代表託管 <c>EntityWorld</c> 已與 Native 打通。
	/// </summary>
	public const uint EntityServiceAbiToken = 0x4E4E456Eu;
}
