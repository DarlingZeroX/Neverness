/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*
* 中文（Phase 8）：本头文件迁至 **VGGalgameContract**，导出宏名保持 `VG_GALGAME_CORE_*`
* 以便与既有 `VG_GALGAME_CORE_API` 标注的 ABI 兼容；物理 DLL 由 **VGGalgameRuntimeCore** 构建并导出符号。
*/

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef VG_GALGAME_CORE_EXPORT
#define VG_GALGAME_CORE_API __declspec(dllexport)
#else
#define VG_GALGAME_CORE_API __declspec(dllimport)
#endif

#else

#ifdef VG_GALGAME_CORE_EXPORT
#define VG_GALGAME_CORE_API __attribute__((visibility("default")))
#else
#define VG_GALGAME_CORE_API
#endif

#endif
