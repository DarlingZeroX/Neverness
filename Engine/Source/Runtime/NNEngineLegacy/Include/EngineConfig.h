#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef ENGINE_API_EXPORT
#define VG_ENGINE_API __declspec(dllexport)
#else
#define VG_ENGINE_API __declspec(dllimport)
#endif

#else
#ifdef ENGINE_API_EXPORT
#define VG_ENGINE_API __attribute__((visibility("default")))
#else
#define VG_ENGINE_API
#endif
#endif

