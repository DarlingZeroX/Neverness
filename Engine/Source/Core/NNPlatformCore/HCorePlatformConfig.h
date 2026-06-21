/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once

#ifdef HCORE_PLATFORM_DYNAMIC
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef H_CORE_PLATFORM_EXPORT
#define H_CORE_PLATFORM_API __declspec(dllexport)
#else
#define H_CORE_PLATFORM_API __declspec(dllimport)
#endif

#else

#ifdef H_CORE_PLATFORM_EXPORT
#define H_CORE_PLATFORM_API __attribute__((visibility("default")))
#else
#define H_CORE_PLATFORM_API
#endif

#endif
#else
#define H_CORE_PLATFORM_API
#endif

// 文件系统 API（静态库模式下为空，原 NNFileSystem 已合并到 NNPlatformCore）
#ifdef H_FILE_SYSTEM_DYNAMIC
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifdef H_FILE_SYSTEM_EXPORT
#define H_FILE_SYSTEM_API __declspec(dllexport)
#else
#define H_FILE_SYSTEM_API __declspec(dllimport)
#endif
#else
#ifdef H_FILE_SYSTEM_EXPORT
#define H_FILE_SYSTEM_API __attribute__((visibility("default")))
#else
#define H_FILE_SYSTEM_API
#endif
#endif
#else
#define H_FILE_SYSTEM_API
#endif