#pragma once

/**
 * @file UIAPI.h
 * @brief UI **Engine Service** 函數表；語義層級抽象 **RmlUi / 自研 UI**，禁止把 Rml::Element* 等指標穿越 ABI。
 *
 * 字串：`utf8Text` 須為 **以 NUL 結尾** 的 UTF-8；nullptr 表示 no-op。
 * Handle：`element` 為 0 時實作應忽略（Stub 符合此契約）。
 */

#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 設定元素關聯之對話/標題文字（引擎語義，非 Gameplay 劇本層）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNUISetDialogueTextFn)(NNElementHandle element, const char* utf8Text);

/** @brief `visible` 使用 0 表隱藏、非 0 表顯示（與布林等價）。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNUISetElementVisibleFn)(NNElementHandle element, int visible);

typedef struct NNUIAPI
{
	NNUISetDialogueTextFn setDialogueText;
	NNUISetElementVisibleFn setElementVisible;
} NNUIAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
