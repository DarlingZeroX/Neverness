/**
 * @file UiApiStubs.cpp
 * @brief **NNUIAPI** 預設 Stub：對白與元素可見性為 no-op，供離線託管測試路徑。
 */

#include "Common/StubInvokeCounter.h"
#include "Internal/ApiStubBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"

namespace
{
void NN_ENGINE_ABI_STDCALL stub_ui_setDialogueText(NNElementHandle element, const char* utf8Text)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)element;
	(void)utf8Text;
}

void NN_ENGINE_ABI_STDCALL stub_ui_setElementVisible(NNElementHandle element, int visible)
{
	NN::StubRuntime::BumpInvokeCount();
	(void)element;
	(void)visible;
}
} // namespace

extern "C" void NNBuildUiApiStubs(NNUIAPI* api)
{
	if (api == nullptr)
	{
		return;
	}
	api->setDialogueText = &stub_ui_setDialogueText;
	api->setElementVisible = &stub_ui_setElementVisible;
}
