/**
 * @file NativeEngineRuntimeApiTable.cpp
 * @brief Runtime 表：先 **BuildDefault**（Stub），再 **NNBuild*RuntimeApi** 覆写指向 **NNEngineRuntime** 的字段。
 */

#include <mutex>

#include "Internal/RuntimeApiBuilders.h"
#include "ManagedRuntimeBridge.h"
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

	NNNativeEngineApiTable_BuildDefault(outTable);

	NNBuildTimingRuntimeApi(&outTable->timing);
	NNBuildAsyncWaitRuntimeApi(&outTable->asyncWait);
	NNBuildSceneRuntimeApi(&outTable->scene);
	NNBuildEditorSceneRuntimeApi(&outTable->editorScene);
	NNBuildAssetRuntimeApi(&outTable->asset);
	NNBuildObjectRuntimeApi(&outTable->object);
	NNBuildAssetRegistryRuntimeApi(&outTable->assetRegistry);
	NNBuildAssetManagerRuntimeApi(&outTable->assetManager);
	NNBuildAssetCookerRuntimeApi(&outTable->assetCooker);
	NNBuildEntityRuntimeApi(&outTable->entity);
	NNBuildApplicationRuntimeApi(&outTable->application);
	NNBuildWindowRuntimeApi(&outTable->window);
	NNBuildVfsRuntimeApi(&outTable->vfs);
	NNBuildEventRuntimeApi(&outTable->events);

	/* 将 VFS 函数表注入 SceneSubsystem，供序列化/反序列化使用 */
	NN::Runtime::engine::NNEngineRuntime::Instance().Scene().SetVfsApi(&outTable->vfs);
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
	NNEngineRuntimeHost_ClearManagedTickCallback();
	NN::Runtime::engine::NNEngineRuntime::Instance().Shutdown();
}
