/**
 * @file EntityApiStubs.cpp
 * @brief **NNEntityAPI** Stub：ABI 冒煙魔數與 **getRuntimeTick**（Stub 恆為 0）。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/EntityAPI.h"
#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
/**
 * @brief **NNEntityAPI** 首包 Stub：`getServiceAbiToken`。
 *
 * 語義（與 MANAGED **§2.7.1** 一致）：
 * - 僅用於宿主／託管 **ExerciseStubInteropPath** 驗證「**`NNNativeEngineAPI::entity`** 已接線」；
 *   返回值固定為 **NN_ENTITY_SERVICE_ABI_TOKEN**，與 **VisionGal.Managed.Engine.NNNativeEngineApiConstants.EntityServiceAbiToken** 對齊。
 * - **不**表示已存在 Native ECS 世界、**不**與 **NNSceneAPI** 之 **NNEntityHandle**（場景圖）互通。
 * - 每次呼叫仍執行 **BumpInvokeCount**，納入 **NNNativeEngineApi_GetStubInvokeCount** 統計。
 */
std::uint32_t NN_ENGINE_ABI_STDCALL stub_entity_getServiceAbiToken(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return NN_ENTITY_SERVICE_ABI_TOKEN;
}

/** @brief Stub：**getRuntimeTick** 恆為 **0**（無 **EntitySubsystem** 驅動）。 */
std::uint64_t NN_ENGINE_ABI_STDCALL stub_entity_getRuntimeTick(void)
{
	NN::StubRuntime::BumpInvokeCount();
	return 0u;
}
} // namespace

extern "C" void NNBuildEntityApiStubs(NNEntityAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->getServiceAbiToken = &stub_entity_getServiceAbiToken;
	api->getRuntimeTick = &stub_entity_getRuntimeTick;
}
