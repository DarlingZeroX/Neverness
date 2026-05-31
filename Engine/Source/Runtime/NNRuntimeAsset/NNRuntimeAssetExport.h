/**
 * @file NNRuntimeAssetExport.h
 * @brief NNRuntimeAsset SHARED 库 DLL export/import 宏。
 *
 * NN_ASSET_DLL_EXPORT  — CMakeLists.txt 中定义，编译 DLL 时生效
 * NN_ASSET_DYNAMIC     — CMakeLists.txt 中定义，标记为 SHARED 库
 */

#pragma once

#ifdef NN_ASSET_DYNAMIC
	#if defined(_WIN32) || defined(_WIN64)
		#ifdef NN_ASSET_DLL_EXPORT
			#define NN_ASSET_API __declspec(dllexport)
		#else
			#define NN_ASSET_API __declspec(dllimport)
		#endif
	#else
		#ifdef NN_ASSET_DLL_EXPORT
			#define NN_ASSET_API __attribute__((visibility("default")))
		#else
			#define NN_ASSET_API
		#endif
	#endif
#else
	#define NN_ASSET_API
#endif
