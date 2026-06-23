namespace Neverness.Runtime.Engine;

/// <summary>
/// 与 Native NN_NATIVE_ENGINE_API_LAYOUT_VERSION（EngineAPIRegistry.h）数值一致；托管读表前必须相等，否则 EngineNativeApiBootstrap.InstallFromNativeApiTable 拒绝缓存。
/// </summary>
/// <remarks>
/// layout v28：新增 NNDiligentAPI 子表。
/// layout v29：ViewportSurfaceAPI 新增 RenderViewportCommands。
/// layout v30：移除 AssetCookerAPI（已迁移至 C#）。
/// layout v31：移除 TimingAPI / UIAPI / SceneAPI（已迁移至 C# 或废弃）。
/// layout v32：移除 EditorSceneAPI（已迁移至 C# Friflo ECS）。
/// layout v33：新增 ImGuiBackendAPI（ImGui SDL3/Diligent 后端封装）。
/// layout v34：NNDiligentAPI 新增 CreateDeviceForWindow + PresentPrimarySwapChain。
/// layout v35：NNDiligentAPI 新增 CreateDeviceForNativeHandle（绕过 SDL，直接传平台原生句柄）。
/// layout v36：NNDiligentAPI 新增 GetPrimaryRenderDevice；ViewportSurface 移除 WindowRegistry 依赖。
/// </remarks>
public static class NNNativeEngineApiConstants
{
	/// <summary>
	/// 当前 NNNativeEngineAPI 聚合体布局版本；破坏性子表字段变更时递增。当前为 36。
	/// v28：新增 NNDiligentAPI（4 个函数指针）子表。
	/// v29：ViewportSurfaceAPI 新增 RenderViewportCommands（1 个函数指针）。
	/// v30：移除 AssetCookerAPI（资产编译已迁移至纯 C# 实现）。
	/// v31：移除 TimingAPI / UIAPI / SceneAPI（已迁移至 C# 或废弃）。
	/// v32：移除 EditorSceneAPI（已迁移至 C# Friflo ECS）。
	/// v33：新增 ImGuiBackendAPI（ImGui SDL3/Diligent 后端封装，5 个函数指针）。
	/// v34：NNDiligentAPI 新增 CreateDeviceForWindow + PresentPrimarySwapChain（2 个函数指针）。
	/// v35：NNDiligentAPI 新增 CreateDeviceForNativeHandle（1 个函数指针，绕过 SDL 直传原生句柄）。
	/// v36：NNDiligentAPI 新增 GetPrimaryRenderDevice（1 个函数指针）；ViewportSurface 移除 WindowRegistry 依赖。
	/// </summary>
	public const uint LayoutVersion = 36;
}
