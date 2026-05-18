#include <cstring>

#include "NNRuntimeNativeEngineApiStub.h"
#include "ManagedRuntimeServices.h"
#include "NativeAPI.h"

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
#include "NativeEngineRuntimeServices.h"
#endif

extern "C" void NNNativeApiTable_BuildDefault(NNNativeAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	std::memset(outTable, 0, sizeof(NNNativeAPI));
	outTable->apiVersion = NN_NATIVE_API_VERSION;
	outTable->reserved0 = 0;
	// 預設實作：帶診斷計數，便於測試斷言 ABI 鏈路。
	outTable->logInfo = &NNNativeApi_DefaultLogInfo;
	// Phase 3：掛載行程單例 Engine Service ABI（Stub）；指標生命週期與進程靜態表一致。
#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	outTable->engineServices = NNNativeEngineApi_GetRuntimeTable();
#else
	outTable->engineServices = NNNativeEngineApi_GetDefaultTable();
#endif
}

extern "C" void NNNativeApi_RegisterLogInfoOverride(NNNativeLogInfoFn overrideFn)
{
	// Phase 2 占位：未來可在此登記宿主或外掛提供之 LogInfo，再參與 BuildDefault 合併策略。
	(void)overrideFn;
}
