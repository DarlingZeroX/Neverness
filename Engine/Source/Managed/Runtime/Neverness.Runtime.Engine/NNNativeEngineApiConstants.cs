namespace Neverness.Runtime.Engine;

/// <summary>
/// 與 Native <c>NN_NATIVE_ENGINE_API_LAYOUT_VERSION</c>（<c>EngineAPIRegistry.h</c>）數值一致；託管讀表前必須相等，否則 <see cref="Neverness.Runtime.Interop.EngineNativeApiBootstrap.InstallFromNativeApiTable"/> 拒絕快取。
/// </summary>
/// <remarks>
/// **layout v12**：<c>NNSceneAPI</c> 重寫為 ECS 組件操作（createScene/createEntity/addComponent/getComponent 等），NNSceneHandle 改為 uint64，componentTypeId = FNV-1a name hash。
/// </remarks>
public static class NNNativeEngineApiConstants
{
	/// <summary>
	/// 當前 <c>NNNativeEngineAPI</c> 聚合體佈局版本；破壞性子表欄位變更時遞增。當前為 **13**。
	/// </summary>
	public const uint LayoutVersion = 13;

	/// <summary>
	/// 與 Native <c>NN_ENTITY_SERVICE_ABI_TOKEN</c>（<c>EntityAPI.h</c>）一致之服務子表冒煙魔數（ASCII「NNEn」小端）；僅用於驗證 <c>NNEntityAPI</c> 已接線，不代表託管 <c>EntityWorld</c> 已與 Native 打通。
	/// </summary>
	public const uint EntityServiceAbiToken = 0x4E4E456Eu;
}
