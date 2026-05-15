/*
 * VGGalgamePresentation — DLL 导出宏（表现层：渲染管线、转场、对白呈现等）
 *
 * 中文：Phase 8 从 VGGalgame 拆出独立模块，与「运行时状态/脚本」解耦。
 */

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef VG_GALGAME_PRESENTATION_EXPORT
#define VG_GALGAME_PRESENTATION_API __declspec(dllexport)
#else
#define VG_GALGAME_PRESENTATION_API __declspec(dllimport)
#endif

#else

#ifdef VG_GALGAME_PRESENTATION_EXPORT
#define VG_GALGAME_PRESENTATION_API __attribute__((visibility("default")))
#else
#define VG_GALGAME_PRESENTATION_API
#endif

#endif
