/**
 * @file EntityRuntimeApi.cpp
 * @brief **NNEntityAPI** Runtime 轉發至 **NNEngineRuntime::Entity()**（**EntitySubsystem**）。
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNRuntimeEngine/Include/NNEngineRuntime.h"

namespace
{
using NN::Runtime::engine::NNEngineRuntime;

/** @brief ABI 冒煙魔數 + 可觀測 **runtimeTick**（與 Stub 恆 0 區分）。 */
std::uint32_t NN_ENGINE_ABI_STDCALL rt_entity_getServiceAbiToken(void)
{
	return NNEngineRuntime::Instance().Entity().GetServiceAbiToken();
}

std::uint64_t NN_ENGINE_ABI_STDCALL rt_entity_getRuntimeTick(void)
{
	return NNEngineRuntime::Instance().Entity().GetRuntimeTick();
}
} // namespace

extern "C" void NNBuildEntityRuntimeApi(NNEntityAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->getServiceAbiToken = &rt_entity_getServiceAbiToken;
	api->getRuntimeTick = &rt_entity_getRuntimeTick;
}
