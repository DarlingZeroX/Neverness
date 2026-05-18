#pragma once

#ifdef VGCORE_DYNAMIC
#if defined(_WIN32) || defined(_WIN64)
#ifdef VG_VFS_API_EXPORT
#define NN_RUNTIME_VFS_API __declspec(dllexport)
#else
#define NN_RUNTIME_VFS_API __declspec(dllimport)
#endif
#else
#ifdef VG_VFS_API_EXPORT
#define NN_RUNTIME_VFS_API __attribute__((visibility("default")))
#else
#define NN_RUNTIME_VFS_API
#endif
#endif
#else
#define NN_RUNTIME_VFS_API
#endif
