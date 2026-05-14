#pragma once

#if defined(_WIN32)
#if defined(VG_MANAGED_HOST_EXPORT)
#define VG_MANAGED_HOST_API __declspec(dllexport)
#else
#define VG_MANAGED_HOST_API __declspec(dllimport)
#endif
#else
#if defined(VG_MANAGED_HOST_EXPORT)
#define VG_MANAGED_HOST_API __attribute__((visibility("default")))
#else
#define VG_MANAGED_HOST_API
#endif
#endif
