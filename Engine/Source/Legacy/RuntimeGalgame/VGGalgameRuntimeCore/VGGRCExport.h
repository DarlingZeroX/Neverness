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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef VG_GALGAME_RUNTIME_CORE_EXPORT
#define VG_RUNTIME_GALCORE_API __declspec(dllexport)
#else
#define VG_RUNTIME_GALCORE_API __declspec(dllimport)
#endif

#else

#ifdef VG_GALGAME_RUNTIME_CORE_EXPORT
#define VG_RUNTIME_GALCORE_API __attribute__((visibility("default")))
#else
#define VG_RUNTIME_GALCORE_API
#endif

#endif