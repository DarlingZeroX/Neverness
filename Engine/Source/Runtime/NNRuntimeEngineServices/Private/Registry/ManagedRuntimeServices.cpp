#include <cstring>

#include "NNRuntimeNativeEngineApiStub.h"
#include "ManagedRuntimeServices.h"
#include "NativeAPI.h"

#if defined(NEVERNESS_USE_ENGINE_RUNTIME_SERVICES) && NEVERNESS_USE_ENGINE_RUNTIME_SERVICES
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
	// 掛載行程單例 Engine Service ABI（Stub）；指標生命週期與進程靜態表一致。
#if defined(NEVERNESS_USE_ENGINE_RUNTIME_SERVICES) && NEVERNESS_USE_ENGINE_RUNTIME_SERVICES
	outTable->engineServices = NNNativeEngineApi_GetRuntimeTable();
#else
	outTable->engineServices = NNNativeEngineApi_GetDefaultTable();
#endif
}
