/**
 * @file RmlRendererAbi.cpp
 * @brief RmlUI Renderer ABI 实现 — Handle 管理。
 *
 * 提供渲染器 Handle 管理，供 C# Neverness.Runtime.Rmlui 模块调用。
 * 使用新版 RmlRenderer（不依赖 NNRuntimeScene）。
 */

#include "ABI/RmlRendererAbi.h"
#include "Renderer/RmlRenderer.h"

#include <Device/INNRenderDevice.h>

#include <iostream>
#include <unordered_map>
#include <mutex>

namespace
{
/**
 * @brief 全局渲染设备指针（由引擎设置）。
 */
NN::Runtime::Render::INNRenderDevice* g_RenderDevice = nullptr;

/**
 * @brief Handle 管理器。
 *
 * 管理多个 RmlRenderer 实例，使用 Handle 标识。
 */
class RmlRendererHandleManager
{
public:
    static RmlRendererHandleManager& Get()
    {
        static RmlRendererHandleManager instance;
        return instance;
    }

    /**
     * @brief 设置渲染设备（由引擎调用）。
     */
    void SetRenderDevice(NN::Runtime::Render::INNRenderDevice* device)
    {
        g_RenderDevice = device;
    }

    /**
     * @brief 创建渲染器并返回 Handle。
     */
    RmlRendererHandle Create(int width, int height)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!g_RenderDevice)
        {
            std::cerr << "[RmlRendererAbi] 渲染设备为空，无法创建渲染器" << std::endl;
            return InvalidRmlRendererHandle;
        }

        // 创建渲染器
        auto* renderer = new NN::Runtime::Renderer::RmlRenderer();
        if (!renderer->Initialize(g_RenderDevice, width, height))
        {
            std::cerr << "[RmlRendererAbi] RmlRenderer 初始化失败" << std::endl;
            delete renderer;
            return InvalidRmlRendererHandle;
        }

        // 分配 Handle
        RmlRendererHandle handle = ++m_nextHandle;
        m_renderers[handle] = renderer;

        std::cout << "[RmlRendererAbi] 创建渲染器, Handle=" << handle
                  << ", Size=" << width << "x" << height << std::endl;

        return handle;
    }

    /**
     * @brief 销毁渲染器。
     */
    void Destroy(RmlRendererHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_renderers.find(handle);
        if (it == m_renderers.end())
        {
            std::cerr << "[RmlRendererAbi] 无效的 Handle: " << handle << std::endl;
            return;
        }

        auto* renderer = it->second;
        renderer->Shutdown();
        delete renderer;
        m_renderers.erase(it);

        std::cout << "[RmlRendererAbi] 销毁渲染器, Handle=" << handle << std::endl;
    }

    /**
     * @brief 获取渲染器。
     */
    NN::Runtime::Renderer::RmlRenderer* Get(RmlRendererHandle handle)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_renderers.find(handle);
        return it != m_renderers.end() ? it->second : nullptr;
    }

private:
    RmlRendererHandleManager() = default;

    RmlRendererHandle m_nextHandle = 0;
    std::unordered_map<RmlRendererHandle, NN::Runtime::Renderer::RmlRenderer*> m_renderers;
    std::mutex m_mutex;
};

} // namespace

// ── ABI 导出函数 ──

extern "C"
{

RmlRendererHandle RmlRenderer_Create(int width, int height)
{
    return RmlRendererHandleManager::Get().Create(width, height);
}

void RmlRenderer_Destroy(RmlRendererHandle handle)
{
    if (handle == InvalidRmlRendererHandle)
        return;

    RmlRendererHandleManager::Get().Destroy(handle);
}

} // extern "C"
