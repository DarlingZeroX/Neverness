#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef VG_RHI_API_EXPORT
#define VG_RHI_API __declspec(dllexport)
#else
#define VG_RHI_API __declspec(dllimport)
#endif

#else
#ifdef VG_RHI_API_EXPORT
#define VG_RHI_API __attribute__((visibility("default")))
#else
#define VG_RHI_API
#endif
#endif

//#ifdef VG_RHI_API_EXPORT
//#define GLAD_API_CALL_EXPORT 1
//#endif
#ifndef GLAD_API_CALL
#define GLAD_API_CALL VG_RHI_API extern
#endif



