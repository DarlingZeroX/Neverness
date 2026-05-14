#include <cstring>

#include "VGNativeEngineAPI/NativeEngineAPI.h"
#include "VGManagedCore/ManagedRuntimeServices.h"
#include "VGManagedCore/NativeAPI.h"

#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
#include "VGEngineRuntimeServices/NativeEngineRuntimeServices.h"
#endif

extern "C" void VGNativeApiTable_BuildDefault(VGNativeAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	std::memset(outTable, 0, sizeof(VGNativeAPI));
	outTable->apiVersion = VG_NATIVE_API_VERSION;
	outTable->reserved0 = 0;
	// 預設實作：帶診斷計數，便於測試斷言 ABI 鏈路。
	outTable->logInfo = &VGNativeApi_DefaultLogInfo;
	// Phase 3：掛載行程單例 Engine Service ABI（Stub）；指標生命週期與進程靜態表一致。
#if defined(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES) && VISIONGAL_USE_ENGINE_RUNTIME_SERVICES
	outTable->engineServices = VGNativeEngineApi_GetRuntimeTable();
#else
	outTable->engineServices = VGNativeEngineApi_GetDefaultTable();
#endif
}

extern "C" void VGNativeApi_RegisterLogInfoOverride(VGNativeLogInfoFn overrideFn)
{
	// Phase 2 占位：未來可在此登記宿主或外掛提供之 LogInfo，再參與 BuildDefault 合併策略。
	(void)overrideFn;
}
