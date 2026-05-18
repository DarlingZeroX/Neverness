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

#if defined(_WIN32) || defined(_WIN64)

#if defined(VG_PACKAGE_API_EXPORT)
#define VG_PACKAGE_API __declspec(dllexport)
#elif defined(VG_PACKAGE_API_STATIC)
#define VG_PACKAGE_API
#else
#define VG_PACKAGE_API __declspec(dllimport)
#endif

#else

#if defined(VG_PACKAGE_API_EXPORT)
#define VG_PACKAGE_API __attribute__((visibility("default")))
#else
#define VG_PACKAGE_API
#endif

#endif

