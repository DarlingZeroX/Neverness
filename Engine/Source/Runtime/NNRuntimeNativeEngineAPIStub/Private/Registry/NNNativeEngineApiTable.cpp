/**
 * @file NNNativeEngineApiTable.cpp
 * @brief 聚合根建表與行程單例 **NNNativeEngineAPI**（僅調用各子系統 **NNBuild*ApiStubs**，不含 Stub 實作）。
 */

#include <cstring>
#include <mutex>

#include "Internal/ApiStubBuilders.h"
#include "NNRuntimeNativeEngineApiStub.h"

extern "C" void NNNativeEngineApiTable_BuildDefault(NNNativeEngineAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	std::memset(outTable, 0, sizeof(NNNativeEngineAPI));
	outTable->layoutVersion = NN_NATIVE_ENGINE_API_LAYOUT_VERSION;
	outTable->reserved0 = 0;

	NNBuildRenderApiStubs(&outTable->render);
	NNBuildAudioApiStubs(&outTable->audio);
	NNBuildInputApiStubs(&outTable->input);
	NNBuildAsyncWaitApiStubs(&outTable->asyncWait);
	NNBuildApplicationApiStubs(&outTable->application);
	NNBuildWindowApiStubs(&outTable->window);
	NNBuildVfsApiStubs(&outTable->vfs);
	NNBuildEventApiStubs(&outTable->events);
	NNBuildRenderAssetApiStubs(&outTable->renderAsset);
	NNBuildViewportRenderApiStubs(&outTable->viewportRender);
}

namespace
{
	std::once_flag g_engineTableOnce;
	NNNativeEngineAPI g_engineTable{};
	const NNNativeEngineAPI* g_engineTablePtr = nullptr;
} // namespace

extern "C" const NNNativeEngineAPI* NNNativeEngineApi_GetDefaultTable(void)
{
	std::call_once(g_engineTableOnce, [] {
		NNNativeEngineApiTable_BuildDefault(&g_engineTable);
		g_engineTablePtr = &g_engineTable;
	});
	return g_engineTablePtr;
}
