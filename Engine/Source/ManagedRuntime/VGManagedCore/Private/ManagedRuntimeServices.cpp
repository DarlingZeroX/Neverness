#include <cstring>

#include "VGManagedCore/ManagedRuntimeServices.h"
#include "VGManagedCore/NativeAPI.h"

extern "C" void VGNativeApiTable_BuildDefault(VGNativeAPI* outTable)
{
	if (outTable == nullptr)
	{
		return;
	}

	std::memset(outTable, 0, sizeof(VGNativeAPI));
	outTable->apiVersion = VG_NATIVE_API_VERSION;
	outTable->reserved0 = 0;
	// 默认实现：带诊断计数，便于测试断言 ABI 链路。
	outTable->logInfo = &VGNativeApi_DefaultLogInfo;
}

extern "C" void VGNativeApi_RegisterLogInfoOverride(VGNativeLogInfoFn overrideFn)
{
	// Phase 2 占位：未来可在此登记宿主或插件提供的 LogInfo，再参与 BuildDefault 合并策略。
	(void)overrideFn;
}
