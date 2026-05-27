#pragma once

/**
 * @file Renderer2DExport.h
 * @brief NevernessRuntime-Renderer2D 动态库导出宏。
 */

#ifdef _WIN32
    #ifdef NN_RUNTIME_RENDERER2D_EXPORT
        #define NN_RUNTIME_RENDERER2D_API __declspec(dllexport)
    #else
        #define NN_RUNTIME_RENDERER2D_API __declspec(dllimport)
    #endif
#else
    #ifdef NN_RUNTIME_RENDERER2D_EXPORT
        #define NN_RUNTIME_RENDERER2D_API __attribute__((visibility("default")))
    #else
        #define NN_RUNTIME_RENDERER2D_API
    #endif
#endif
