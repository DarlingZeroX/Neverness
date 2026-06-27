#pragma once

/**
 * @file RmlRendererAbi.h
 * @brief RmlUI Renderer ABI — C# 调用入口。
 *
 * 提供渲染器 Handle 管理，供 C# Neverness.Runtime.Rmlui 模块调用。
 * 使用 Handle 而非裸指针，更安全。
 */

#include "../../RuntimeRmlUIExport.h"

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/// 渲染器 Handle 类型
using RmlRendererHandle = std::uint32_t;

/// 无效 Handle 值
constexpr RmlRendererHandle InvalidRmlRendererHandle = 0;

/**
 * @brief 创建 RmlUI 渲染器实例。
 *
 * 内部创建 RmlUIRenderer 并初始化。
 *
 * @param width 视口宽度
 * @param height 视口高度
 * @return 渲染器 Handle，0 表示失败
 */
NN_RUNTIME_RMLUI_API RmlRendererHandle RmlRenderer_Create(int width, int height);

/**
 * @brief 销毁 RmlUI 渲染器实例。
 *
 * @param handle 渲染器 Handle
 */
NN_RUNTIME_RMLUI_API void RmlRenderer_Destroy(RmlRendererHandle handle);

/**
 * @brief 设置渲染器的 RmlUI Context。
 *
 * 将 C# RmlUi.Net 创建的 Context 原生指针传给 C++ 渲染器，
 * 使 C++ 渲染器使用同一个 Context 进行渲染。
 * 调用后会替换掉 Initialize 时创建的内部 Context。
 *
 * @param handle 渲染器 Handle
 * @param contextPtr Rml::Context* 原生指针（从 RmlUi.Net Context.NativePtr 获取）
 */
NN_RUNTIME_RMLUI_API void RmlRenderer_SetContext(RmlRendererHandle handle, void* contextPtr);

#ifdef __cplusplus
} /* extern "C" */
#endif
