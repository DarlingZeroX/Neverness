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
#include "VGEditorCore/Interface/UIInterface.h"
#include "VGEditorCore/Interface/UITaskInterface.h"

#if defined(_WIN32) || defined(_WIN64)
#ifdef VG_EDITOR_FRAMEWORK_API_EXPORT
	#define VG_EDITOR_FRAMEWORK_API __declspec(dllexport)
#else
	#define VG_EDITOR_FRAMEWORK_API __declspec(dllimport)
#endif

#else
#ifdef VG_EDITOR_FRAMEWORK_API_EXPORT
	#define VG_EDITOR_FRAMEWORK_API __attribute__((visibility("default")))
#else
	#define VG_EDITOR_FRAMEWORK_API
#endif
#endif



