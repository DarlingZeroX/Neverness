#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef H_NG_RUNTIME_NODES_API_EXPORT
#define H_NG_RUNTIME_NODES_API __declspec(dllexport)
#else
#define H_NG_RUNTIME_NODES_API __declspec(dllimport)
#endif

#else
#ifdef H_NG_RUNTIME_NODES_API_EXPORT
#define H_NG_RUNTIME_NODES_API __attribute__((visibility("default")))
#else
#define H_NG_RUNTIME_NODES_API
#endif
#endif

