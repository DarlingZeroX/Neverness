#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef H_MEDIA_API_EXPORT
#define H_MEDIA_API __declspec(dllexport)
#else
#define H_MEDIA_API __declspec(dllimport)
#endif

#else
#ifdef H_MEDIA_API_EXPORT
#define H_MEDIA_API __attribute__((visibility("default")))
#else
#define H_MEDIA_API
#endif
#endif

