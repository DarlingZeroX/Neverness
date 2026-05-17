#pragma once

#ifdef VGCORE_DYNAMIC
#if defined(_WIN32) || defined(_WIN64)
#ifdef VG_CORE_API_EXPORT
#define NN_RUNTIME_CORE_API __declspec(dllexport)
#else
#define NN_RUNTIME_CORE_API __declspec(dllimport)
#endif

#else
#ifdef VG_CORE_API_EXPORT
#define NN_RUNTIME_CORE_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_CORE_API
#endif
#endif
#else
#define NN_RUNTIME_CORE_API 
#endif

