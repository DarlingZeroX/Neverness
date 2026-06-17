#pragma once

/**
 * @file RmlUISingletonAccess.h
 * @brief RmlUI 单例 getter 函数声明——供 ViewportSurfaceRuntimeApi 访问 ViewportRenderRuntimeApi 的 RmlUI 实例。
 *
 * 生命周期：RmlUI 单例由 ViewportRenderRuntimeApi 创建和销毁。
 * 其他模块通过 getter 获取观察指针，不持有所有权。
 */

// 前向声明
namespace NN::Runtime::Renderer { class RmlUIRenderer; }
namespace NN::Runtime::RmlUI { class NNRmlUISystem; }

/// 获取 RmlUI 渲染器单例（由 ViewportRenderRuntimeApi 管理生命周期）。
NN::Runtime::Renderer::RmlUIRenderer* GetRmlUIRenderer();

/// 获取 RmlUI 系统单例（由 ViewportRenderRuntimeApi 管理生命周期）。
NN::Runtime::RmlUI::NNRmlUISystem* GetRmlUISystem();
