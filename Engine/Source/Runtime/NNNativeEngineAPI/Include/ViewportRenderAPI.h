#pragma once

/**
 * @file ViewportRenderAPI.h
 * @brief 视口渲染 API：C# EditorViewport 调用以渲染场景到离屏 Framebuffer。
 *
 * 所有函数使用 __stdcall（Windows）；未接线时回传 0 / no-op。
 * v21 新增子表。
 */

#include "NativeInterop.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NNViewportRenderAPI
{
    /**
     * @brief 渲染场景到离屏 Framebuffer，返回 OpenGL Texture ID。
     * @param sceneHandle  场景句柄（NNSceneHandle）
     * @param width        视口宽度
     * @param height       视口高度
     * @return OpenGL Texture ID（0 = 失败）
     */
    std::uint64_t (NN_ENGINE_ABI_STDCALL *RenderSceneToTexture)(
        std::uint64_t sceneHandle,
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 查询上次渲染的 Texture ID（不重新渲染）。
     * @return OpenGL Texture ID（0 = 未渲染过）
     */
    std::uint64_t (NN_ENGINE_ABI_STDCALL *GetLastRenderedTexture)(void);

    /**
     * @brief 获取渲染统计信息。
     * @param outDrawCalls  输出 DrawCall 数量（可为 nullptr）
     * @param outQuadCount  输出 Quad 数量（可为 nullptr）
     */
    void (NN_ENGINE_ABI_STDCALL *GetRenderStats)(
        std::uint32_t* outDrawCalls,
        std::uint32_t* outQuadCount);

} NNViewportRenderAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
