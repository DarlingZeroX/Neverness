#pragma once

/**
 * @file AssetManagerAPI.h
 * @brief Runtime 資產管理器 ABI 函數表（同步/異步載入、卸載、查詢、包管理）。
 *
 * 與 AssetAPI.h（舊版紋理/音訊快捷載入）分離；本表為 NNRuntimeAsset 模組之完整介面。
 * 所有函數使用 __stdcall（Windows）；未接線時回傳 0 / nullptr。
 */

#include "EngineTypes.h"
#include "EngineHandles.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 回調型別 ======================== */

/** @brief 資產載入完成回調。userData 為呼叫端傳入之不透明指標。 */
typedef void(NN_ENGINE_ABI_STDCALL* NNAssetLoadCompletedCallback)(
	NNAssetHandle handle,
	int result,        /* 0 = 成功，非 0 = 失敗 */
	void* userData);

/* ======================== 載入優先級 ======================== */

typedef enum NNLoadPriority
{
	NN_LOAD_PRIORITY_CRITICAL   = 0,  /* 立即需要（UI 紋理等） */
	NN_LOAD_PRIORITY_HIGH       = 1,  /* 相機附近資源 */
	NN_LOAD_PRIORITY_NORMAL     = 2,  /* 預設 */
	NN_LOAD_PRIORITY_LOW        = 3,  /* 後台預載 */
	NN_LOAD_PRIORITY_BACKGROUND = 4   /* 最低優先級 */
} NNLoadPriority;

/* ======================== 函數表 ======================== */

typedef struct NNAssetManagerAPI
{
	/* ---------- 同步載入 ---------- */

	/** @brief 同步載入資產，回傳 Handle（阻塞直到載入完成）。 */
	NNAssetHandle(NN_ENGINE_ABI_STDCALL *loadAssetSync)(
		NNGuid guid,
		std::uint64_t typeId);

	/* ---------- 異步載入 ---------- */

	/** @brief 異步載入資產，回傳 AsyncWaitHandle（輪詢完成）。 */
	NNAsyncWaitHandle(NN_ENGINE_ABI_STDCALL *loadAssetAsync)(
		NNGuid guid,
		std::uint64_t typeId,
		NNLoadPriority priority,
		NNAssetLoadCompletedCallback callback,
		void* callbackUserData);

	/* ---------- 卸載 ---------- */

	/** @brief 卸載資產（引用計數 -1，計數為 0 時釋放）。 */
	void(NN_ENGINE_ABI_STDCALL *unloadAsset)(NNAssetHandle handle);

	/** @brief 依 GUID 卸載資產。 */
	void(NN_ENGINE_ABI_STDCALL *unloadAssetByGuid)(NNGuid guid);

	/* ---------- 查詢 ---------- */

	/** @brief 資產是否已載入。 */
	int(NN_ENGINE_ABI_STDCALL *isAssetLoaded)(NNAssetHandle handle);

	/** @brief 資產是否正在載入中。 */
	int(NN_ENGINE_ABI_STDCALL *isAssetLoading)(NNAssetHandle handle);

	/** @brief 依 GUID 取得已載入資產之 Handle（未載入回傳 0）。 */
	NNAssetHandle(NN_ENGINE_ABI_STDCALL *getAssetByGuid)(NNGuid guid);

	/** @brief 取得資產之 GUID（Handle 無效回傳全零 GUID）。 */
	NNGuid(NN_ENGINE_ABI_STDCALL *getGuidByAsset)(NNAssetHandle handle);

	/* ---------- 引用計數 ---------- */

	/** @brief 增加引用計數。 */
	void(NN_ENGINE_ABI_STDCALL *addRef)(NNAssetHandle handle);

	/** @brief 減少引用計數。 */
	void(NN_ENGINE_ABI_STDCALL *releaseRef)(NNAssetHandle handle);

	/** @brief 取得當前引用計數。 */
	std::uint32_t(NN_ENGINE_ABI_STDCALL *getRefCount)(NNAssetHandle handle);

	/* ---------- 資料存取 ---------- */

	/** @brief 取得資產原始資料指標（未載入回傳 nullptr）。 */
	const void*(NN_ENGINE_ABI_STDCALL *getAssetData)(NNAssetHandle handle);

	/** @brief 取得資產原始資料大小。 */
	std::uint64_t(NN_ENGINE_ABI_STDCALL *getAssetDataSize)(NNAssetHandle handle);

	/* ---------- Blob 存取 ---------- */

	/** @brief 取得資產中 blob 數量。 */
	std::uint32_t(NN_ENGINE_ABI_STDCALL *getBlobCount)(NNAssetHandle handle);

	/** @brief 取得指定 blob 資料指標。 */
	const void*(NN_ENGINE_ABI_STDCALL *getBlobData)(NNAssetHandle handle, std::uint32_t index);

	/** @brief 取得指定 blob 大小。 */
	std::uint64_t(NN_ENGINE_ABI_STDCALL *getBlobSize)(NNAssetHandle handle, std::uint32_t index);

	/* ---------- 包管理 ---------- */

	/** @brief 掛載 .nnpack 包（成功回傳 0）。 */
	int(NN_ENGINE_ABI_STDCALL *mountPackage)(const char* packPathUtf8);

	/** @brief 卸載 .nnpack 包。 */
	void(NN_ENGINE_ABI_STDCALL *unmountPackage)(const char* packPathUtf8);

	/** @brief 查詢資產是否在已掛載包中。 */
	int(NN_ENGINE_ABI_STDCALL *isAssetInPackage)(NNGuid guid);

	/* ---------- Hot Reload ---------- */

	/** @brief 標記資產需要重載（Editor 通知 Runtime 用）。 */
	void(NN_ENGINE_ABI_STDCALL *markForReload)(NNGuid guid);

	/** @brief 重新載入所有被標記的資產。 */
	void(NN_ENGINE_ABI_STDCALL *reloadMarkedAssets)(void);

	/* ---------- 統計 ---------- */

	/** @brief 取得已載入資產數量。 */
	std::uint64_t(NN_ENGINE_ABI_STDCALL *getLoadedAssetCount)(void);

	/** @brief 取得資產系統總記憶體使用量（位元組）。 */
	std::uint64_t(NN_ENGINE_ABI_STDCALL *getTotalMemoryUsage)(void);

} NNAssetManagerAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
