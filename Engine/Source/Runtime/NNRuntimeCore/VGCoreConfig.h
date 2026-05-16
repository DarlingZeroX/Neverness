#pragma once

#ifdef VGCORE_DYNAMIC
#if defined(_WIN32) || defined(_WIN64)
#ifdef VG_CORE_API_EXPORT
#define VG_CORE_API __declspec(dllexport)
#else
#define VG_CORE_API __declspec(dllimport)
#endif

#else
#ifdef VG_CORE_API_EXPORT
#define VG_CORE_API __attribute__((visibility("default")))
#else
#define VG_CORE_API
#endif
#endif
#else
#define VG_CORE_API 
#endif

