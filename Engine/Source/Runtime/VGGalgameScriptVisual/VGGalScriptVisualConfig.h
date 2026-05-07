/*
* VGGalgameRuntime 模块导出宏
*
* 目标：
* - 提供“纯运行时逻辑”库（节点执行函数等）
* - Editor 模块只负责注册与 UI，不包含执行实现
*/
#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef VG_GALGAME_SCRIPT_VISUAL_EXPORT
#define VG_GALGAME_SCRIPT_VISUAL_API __declspec(dllexport)
#else
#define VG_GALGAME_SCRIPT_VISUAL_API __declspec(dllimport)
#endif

#else

#ifdef VG_GALGAME_SCRIPT_VISUAL_EXPORT
#define VG_GALGAME_SCRIPT_VISUAL_API __attribute__((visibility("default")))
#else
#define VG_GALGAME_SCRIPT_VISUAL_API
#endif

#endif

