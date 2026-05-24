#pragma once

/**
 * @file AssetCookerAPI.h
 * @brief 资产编译/打包器 ABI 函数表。
 *
 * 提供给 C# BuildPipeline 调用，编排构建流程。
 */

#include "EngineTypes.h"
#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 构建结果 ======================== */

typedef struct NNCookResultData
{
	int          success;
	std::uint32_t totalAssets;
	std::uint32_t cookedAssets;
	std::uint32_t failedAssets;
	std::uint32_t generatedPacks;
	double       elapsedSeconds;
} NNCookResultData;

/* ======================== 函数表 ======================== */

typedef struct NNAssetCookerAPI
{
	/** @brief 创建构建清单，返回清单句柄。 */
	std::uint64_t(NN_ENGINE_ABI_STDCALL *createManifest)(void);

	/** @brief 销毁构建清单。 */
	void(NN_ENGINE_ABI_STDCALL *destroyManifest)(std::uint64_t manifestHandle);

	/** @brief 设置清单的输出根目录。 */
	void(NN_ENGINE_ABI_STDCALL *setOutputRoot)(std::uint64_t manifestHandle, const char* pathUtf8);

	/** @brief 设置清单的 Library 根目录。 */
	void(NN_ENGINE_ABI_STDCALL *setLibraryRoot)(std::uint64_t manifestHandle, const char* pathUtf8);

	/** @brief 添加待编译资产到清单。 */
	void(NN_ENGINE_ABI_STDCALL *addAsset)(std::uint64_t manifestHandle,
		NNGuid guid, std::uint64_t typeId, const char* sourcePathUtf8, std::uint32_t groupIndex);

	/** @brief 添加构建分组到清单。 */
	void(NN_ENGINE_ABI_STDCALL *addGroup)(std::uint64_t manifestHandle,
		const char* nameUtf8, const char* addressUtf8, std::uint32_t strategy, const char* outputPathUtf8);

	/** @brief 执行构建，返回结果。 */
	NNCookResultData(NN_ENGINE_ABI_STDCALL *cook)(std::uint64_t manifestHandle);

} NNAssetCookerAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
