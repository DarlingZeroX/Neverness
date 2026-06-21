/**
 * @file NativeEngineRuntimeApiTable.cpp
 * @brief Runtime 表：先 **BuildDefault**（Stub），再 **NNBuild*RuntimeApi** 覆写指向 **NNEngineRuntime** 的字段。
 */

#include <iostream>
#include <mutex>

#include "Internal/RuntimeApiBuilders.h"
//#include "ManagedRuntimeBridge.h"
#include "ApplicationApiExport.h"
#include "EventApiExport.h"
#include "WindowApiExport.h"
#include "VfsApiExport.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"
#include "NNRuntimeNativeEngineApiStub.h"
#include "NativeEngineRuntimeServices.h"

extern "C" void NNNativeEngineApiTable_BuildRuntime(NNNativeEngineAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	//NNNativeEngineApiTable_BuildDefault(outTable);
	std::cout << "---------------------------------------" << std::endl;
	std::cout << "Building NNNative Engine Api Table..." << std::endl;
	outTable->layoutVersion = NN_NATIVE_ENGINE_API_LAYOUT_VERSION;
	outTable->reserved0 = 0;

	NNBuildTimingRuntimeApi(&outTable->timing);
	NNBuildAsyncWaitRuntimeApi(&outTable->asyncWait);
	//NNBuildSceneRuntimeApi(&outTable->scene);           // 已移除：SceneRuntimeApi 随 NNRuntimeScene 移至 Legacy
	//NNBuildEditorSceneRuntimeApi(&outTable->editorScene); // 已移除：同上
	//NNBuildAssetCookerRuntimeApi(&outTable->assetCooker); // 已移除：已迁移至 C#
	NNBuildApplicationRuntimeApi(&outTable->application);
	NNBuildWindowRuntimeApi(&outTable->window);
	NNBuildVfsRuntimeApi(&outTable->vfs);
	NNBuildEventRuntimeApi(&outTable->events);
	//NNBuildRenderAssetRuntimeApi(&outTable->renderAsset);  // 已移除：RenderAssetRuntimeApi 随 NNRenderAssets 移至 Legacy
	NNBuildViewportRenderRuntimeApi(&outTable->viewportRender);
	NNBuildViewportSurfaceRuntimeApi(&outTable->viewportSurface);
	NNBuildDiligentRuntimeApi(&outTable->diligent);

	std::cout << "NNNative Engine Api Table built." << std::endl;
	std::cout << "---------------------------------------" << std::endl;
}

namespace
{
	std::once_flag g_runtimeTableOnce;
	NNNativeEngineAPI g_runtimeTable{};
	const NNNativeEngineAPI* g_runtimeTablePtr = nullptr;
} // namespace

extern "C" const NNNativeEngineAPI* NNNativeEngineApi_GetRuntimeTable(void)
{
	std::call_once(g_runtimeTableOnce, [] {
		NNNativeEngineApiTable_BuildRuntime(&g_runtimeTable);
		g_runtimeTablePtr = &g_runtimeTable;
	});
	return g_runtimeTablePtr;
}

extern "C" bool NNEngineRuntimeHost_Initialize(void)
{
	return NN::Runtime::engine::NNEngineRuntime::Instance().Initialize();
}

extern "C" void NNEngineRuntimeHost_Tick(float deltaTimeSeconds)
{
	NN::Runtime::engine::NNEngineRuntime::Instance().Tick(deltaTimeSeconds);
}

extern "C" void NNEngineRuntimeHost_Shutdown(void)
{
	ShutdownViewportSurface();
	ShutdownViewportRender();
	NN::Runtime::engine::NNEngineRuntime::Instance().Shutdown();
}
