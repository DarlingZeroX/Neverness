#pragma once

/**
 * @file ViewportRenderAPI.h
 * @brief 视口渲染 API：RmlUI 渲染管理。
 *
 * 所有函数使用 __stdcall（Windows）；未接线时回传 0 / no-op。
 * v21 新增子表。
 *
 * 已移除：RenderSceneToTexture / GetLastRenderedTexture / GetRenderStats
 * （依赖 C++ Legacy Scene，已不需要）
 */

#include "NativeInterop.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NNViewportRenderAPI
{
    /**
     * @brief 设置 RmlUI 视口尺寸。
     * @param width   视口宽度
     * @param height  视口高度
     */
    void (NN_ENGINE_ABI_STDCALL *SetRmlUIViewportSize)(
        std::uint32_t width,
        std::uint32_t height);

    /**
     * @brief 处理 RmlUI 输入事件。
     * @param type      事件类型（0=MouseMove, 1=MouseButtonDown, 2=MouseButtonUp, 3=MouseWheel, 4=KeyDown, 5=KeyUp）
     * @param mouseX    鼠标 X
     * @param mouseY    鼠标 Y
     * @param wheelX    滚轮 X
     * @param wheelY    滚轮 Y
     * @param button    鼠标按钮
     * @param keyCode   键盘按键码
     * @param keyMod    键盘修饰符
     */
    void (NN_ENGINE_ABI_STDCALL *ProcessRmlUIInput)(
        std::uint32_t type,
        std::int32_t mouseX, std::int32_t mouseY,
        std::int32_t wheelX, std::int32_t wheelY,
        std::uint32_t button,
        std::uint32_t keyCode, std::uint32_t keyMod);

    /**
     * @brief 获取上次 RmlUI 渲染的纹理 ID（不重新渲染）。
     * @return 纹理 ID（0 = 未渲染过或无 RmlUI 内容）
     */
    std::uint64_t (NN_ENGINE_ABI_STDCALL *GetLastRmluiTexture)(void);

    /**
     * @brief 通知 native 端重新加载指定文档（热重载）。
     * @param vfsPath 文档的 VFS 路径（UTF-8，NUL 终结）
     */
    void (NN_ENGINE_ABI_STDCALL *ReloadRmlDocument)(
        const char* vfsPath);

    /**
     * @brief 通知 native 端重新加载所有文档（热重载）。
     */
    void (NN_ENGINE_ABI_STDCALL *ReloadAllRmlDocuments)(void);

} NNViewportRenderAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
