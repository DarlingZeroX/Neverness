/**
 * @file ViewportSurfaceRuntimeApi.cpp
 * @brief NNViewportSurfaceAPI Runtime 实现：管理多个 ViewportSurface 的 SwapChain。
 *
 * 设计：
 * - 每个 Surface 持有独立 SwapChain（复用主窗口的 Device/Context）
 * - Deferred Resize：MarkResize → FlushResizes（帧末统一执行）
 * - Surface Lost：HandleDestroyed → MarkLost → HandleCreated → Recreate
 * - 进程级 Surface 注册表，monotonic ID 分配
 * - v29：新增 RenderViewportCommands——C# Scene → Flat Buffer → C++ Renderer
 */

#include "Internal/RuntimeApiBuilders.h"

#include "NNNativeEngineAPI/Include/NativeInterop.h"
#include "NNNativeEngineAPI/Include/ViewportSurfaceAPI.h"
#include "NNNativeEngineAPI/Include/RenderCommands.h"
#include "Core/WindowRegistry.h"
#include <Device/INNRenderDevice.h>

// NNRuntimeDiligent（NNDiligentViewportSurface、NNDiligentDevice）
#include "Device/NNDiligentViewportSurface.h"
#include "Device/NNDiligentDevice.h"

// Renderer2D（RenderCommands 路径直接使用）
#include "NNRuntimeRenderer2D/Include/Renderer2D/Renderer2D.h"
#include "NNRuntimeRenderer2D/Include/Renderer2D/FramebufferObject.h"
#include "NNRuntimeRenderer2D/Include/Renderer2D/CameraData.h"
#include "NNRuntimeRenderer2D/Include/Renderer2D/SpriteDrawCommand.h"

// RmlUI（Overlay Pass）
#include "NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUISystem.h"
#include "NNRuntimeRmlui/Include/System/NNRmlUIModule.h"

// Diligent（CopyTexture / ITextureView）
#include "NNDiligentConfig.h"
#include <Texture.h>

#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstring>

#include "NativeEngineRuntimeServices.h"
#include "NNCore/Interface/HLog.h"

// RmlUI 单例 getter 函数声明
#include "Internal/RmlUISingletonAccess.h"

namespace
{
    // ── Surface 注册表 ──

    std::mutex g_SurfaceMutex;
    std::atomic<std::uint64_t> g_NextSurfaceId{1};
    std::unordered_map<std::uint64_t, NNDiligent::NNDiligentViewportSurface*> g_Surfaces;

    // 延迟 Resize 条目
    struct ResizeEntry
    {
        std::uint64_t surfaceId;
        std::uint32_t width;
        std::uint32_t height;
    };
    std::vector<ResizeEntry> g_PendingResizes;

    /// 查找 Surface
    NNDiligent::NNDiligentViewportSurface* FindSurface(std::uint64_t surfaceId)
    {
        auto it = g_Surfaces.find(surfaceId);
        return (it != g_Surfaces.end()) ? it->second : nullptr;
    }

    // ── API 实现 ──

    std::uint64_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_createSurface(
        void* nativeHandle,
        NNNativeHandleType handleType,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (!nativeHandle)
        {
            std::cerr << "[ViewportSurface] CreateSurface 参数无效: nativeHandle 为空" << std::endl;
            return 0;
        }

        // 最小尺寸保护（避免创建 0x0 SwapChain）
        if (width < 1) width = 1;
        if (height < 1) height = 1;

        // 获取主窗口的 Diligent Device/Context
        auto primaryHandle = NN::Runtime::WindowRegistry::GetPrimaryHandle();
        if (primaryHandle == 0)
        {
            std::cerr << "[ViewportSurface] 主窗口未创建" << std::endl;
            return 0;
        }

        auto* window = NN::Runtime::WindowRegistry::Resolve(primaryHandle);
        if (!window)
        {
            std::cerr << "[ViewportSurface] 主窗口无法解析" << std::endl;
            return 0;
        }

        // 通过 VGWindow 获取 INNRenderDevice，cast 为 NNDiligentDevice
        // Neverness 目前只使用 NNDiligent 作为渲染后端，此 cast 安全
        auto* iDevice = static_cast<NN::Runtime::Render::INNRenderDevice*>(window->GetDevice());
        if (!iDevice)
        {
            std::cerr << "[ViewportSurface] Diligent 设备未初始化" << std::endl;
            return 0;
        }

        auto* diliDevice = static_cast<NNDiligent::NNDiligentDevice*>(iDevice);
        auto* device  = diliDevice->GetDiligentDevice();
        auto* context = diliDevice->GetDiligentContext();
        if (!device || !context)
        {
            std::cerr << "[ViewportSurface] Diligent Device/Context 无效" << std::endl;
            return 0;
        }

        // 创建 Surface
        auto* surface = new NNDiligent::NNDiligentViewportSurface();
        if (!surface->Create(device, context, nativeHandle, static_cast<uint32_t>(handleType), width, height))
        {
            std::cerr << "[ViewportSurface] SwapChain 创建失败" << std::endl;
            delete surface;
            return 0;
        }

        // 分配 surfaceId 并注册
        std::uint64_t surfaceId = g_NextSurfaceId.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(g_SurfaceMutex);
            g_Surfaces[surfaceId] = surface;
        }

        std::cout << "[ViewportSurface] Surface " << surfaceId << " 已创建: "
                  << width << "x" << height << std::endl;
        return surfaceId;
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_destroySurface(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto it = g_Surfaces.find(surfaceId);
        if (it != g_Surfaces.end())
        {
            it->second->Destroy();
            delete it->second;
            g_Surfaces.erase(it);
            std::cout << "[ViewportSurface] Surface " << surfaceId << " 已销毁" << std::endl;
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_markResize(
        std::uint64_t surfaceId,
        std::uint32_t width,
        std::uint32_t height)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (surface)
        {
            g_PendingResizes.push_back({surfaceId, width, height});
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_flushResizes()
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);

        // 合并同一 surface 的多次 resize（只保留最后一次）
        std::unordered_map<std::uint64_t, ResizeEntry> merged;
        for (const auto& entry : g_PendingResizes)
        {
            merged[entry.surfaceId] = entry;
        }
        g_PendingResizes.clear();

        // 执行所有 resize
        for (const auto& [id, entry] : merged)
        {
            auto* surface = FindSurface(id);
            if (surface)
            {
                surface->MarkResize(entry.width, entry.height);
                surface->FlushResize();
            }
        }
    }

    void NN_ENGINE_ABI_STDCALL rt_viewportSurface_present(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (surface)
        {
            surface->Present();
        }
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_isSurfaceLost(std::uint64_t surfaceId)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        return surface ? (surface->IsSurfaceLost() ? 1 : 0) : 0;
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_recreateSurface(
        std::uint64_t surfaceId,
        void* newHandle,
        NNNativeHandleType newHandleType)
    {
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (!surface)
            return 0;

        return surface->Recreate(newHandle, static_cast<uint32_t>(newHandleType)) ? 1 : 0;
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_renderViewport(
        std::uint64_t surfaceId,
        std::uint64_t sceneHandle,
        std::uint32_t width,
        std::uint32_t height)
    {
        if (surfaceId == 0 || sceneHandle == 0 || width == 0 || height == 0)
            return 0;

        // 0. 获取 SwapChain
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (!surface || !surface->IsCreated())
            return 0;

        auto* swapChain = surface->GetSwapChain();
        if (!swapChain)
            return 0;

        // 1. 渲染场景到离屏 FBO（通过 ViewportRender API）
        //    内部会：SceneRenderer::Render → EndRenderPass → RmlUI::CompositeOnTop → EndRenderPass
        const auto* api = NNNativeEngineApi_GetRuntimeTable();
        if (!api || !api->viewportRender.RenderSceneToTexture)
            return 0;

        std::uint64_t sceneTextureId = api->viewportRender.RenderSceneToTexture(sceneHandle, width, height);
        if (sceneTextureId == 0)
            return 0;

        // 2. 确保 SwapChain 尺寸与渲染目标一致
        auto& swapChainDesc = swapChain->GetDesc();
        if (swapChainDesc.Width != width || swapChainDesc.Height != height)
        {
            swapChain->Resize(width, height);
        }

        // 3. 获取离屏纹理（ITextureView* 编码为 uint64_t）
        auto* srcTextureView = reinterpret_cast<::Diligent::ITextureView*>(sceneTextureId);
        if (!srcTextureView)
            return 0;

        auto* srcTexture = srcTextureView->GetTexture();
        if (!srcTexture)
            return 0;

        // 4. 获取 SwapChain back buffer 纹理
        auto* dstRTV = swapChain->GetCurrentBackBufferRTV();
        if (!dstRTV)
            return 0;

        auto* dstTexture = dstRTV->GetTexture();
        if (!dstTexture)
            return 0;

        // 5. CopyTexture（FBO → SwapChain back buffer）+ Present
        //    CopyTexture 和 Present 在同一个命令列表中，一起提交
        auto* context = surface->GetContext();
        if (!context)
            return 0;

        ::Diligent::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = srcTexture;
        copyAttribs.SrcTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.pDstTexture = dstTexture;
        copyAttribs.DstTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        context->CopyTexture(copyAttribs);

        // 6. Present（内部会 Flush + Close 命令列表）
        swapChain->Present();

        return 1;
    }

    // ═══════════════════════════════════════════
    //  RenderViewportCommands 相关单例
    // ═══════════════════════════════════════════

    // Renderer2D 和 FramebufferObject——惰性初始化
    NN::Runtime::Renderer2D::Renderer2D* g_CommandsRenderer = nullptr;
    NN::Runtime::Renderer2D::FramebufferObject* g_CommandsFBO = nullptr;
    bool g_CommandsRendererInitialized = false;

    // RmlUI 单例引用——通过 RmlUISingletonAccess.h 中声明的 getter 函数获取
    // 注意：RmlUI 的生命周期由 ViewportRenderRuntimeApi 管理

    /// 确保 RenderCommands 路径的 Renderer2D 已初始化
    bool EnsureCommandsRenderer()
    {
        if (g_CommandsRendererInitialized)
            return g_CommandsRenderer != nullptr;

        // 从主窗口获取 Diligent 设备
        NN::Runtime::Render::INNRenderDevice* device = nullptr;
        if (auto* window = NN::Runtime::WindowRegistry::Resolve(
                NN::Runtime::WindowRegistry::GetPrimaryHandle()))
        {
            device = window->GetDevice();
        }
        if (!device)
        {
            std::cerr << "[ViewportSurface] RenderCommands: 无法获取 Diligent 设备" << std::endl;
            return false;
        }

        g_CommandsRenderer = new NN::Runtime::Renderer2D::Renderer2D();
        if (!g_CommandsRenderer->Initialize(device))
        {
            std::cerr << "[ViewportSurface] RenderCommands: Renderer2D 初始化失败" << std::endl;
            delete g_CommandsRenderer;
            g_CommandsRenderer = nullptr;
            g_CommandsRendererInitialized = true;
            return false;
        }

        g_CommandsFBO = new NN::Runtime::Renderer2D::FramebufferObject();
        if (!g_CommandsFBO->Initialize(device, 1280, 720))
        {
            std::cerr << "[ViewportSurface] RenderCommands: FramebufferObject 初始化失败" << std::endl;
            g_CommandsRenderer->Shutdown();
            delete g_CommandsRenderer;
            g_CommandsRenderer = nullptr;
            delete g_CommandsFBO;
            g_CommandsFBO = nullptr;
            g_CommandsRendererInitialized = true;
            return false;
        }

        g_CommandsRendererInitialized = true;
        std::cout << "[ViewportSurface] RenderCommands: Renderer2D + FBO 初始化成功" << std::endl;
        return true;
    }

    /// NNSpriteInstance → SpriteDrawCommand 转换
    void ConvertToSpriteDrawCommand(const NNSpriteInstance& src,
                                    NN::Runtime::Renderer2D::SpriteDrawCommand& dst)
    {
        std::memcpy(dst.Transform, src.transform, sizeof(float) * 16);
        dst.TextureHandle = src.textureHandle;
        std::memcpy(dst.Color, src.color, sizeof(float) * 4);
        std::memcpy(dst.UvRect, src.uvRect, sizeof(float) * 4);
        dst.Layer     = src.layer;
        dst.SortOrder = src.sortOrder;
        dst.Blend     = static_cast<NN::Runtime::Renderer2D::BlendMode>(src.blendMode);
        dst.FlipX     = (src.flags & NN_SPRITE_FLAG_FLIP_X) != 0;
        dst.FlipY     = (src.flags & NN_SPRITE_FLAG_FLIP_Y) != 0;
    }

    /// NNSetCameraData → CameraData 转换
    void ConvertToCameraData(const NNSetCameraData& src,
                             NN::Runtime::Renderer2D::CameraData& dst)
    {
        std::memcpy(dst.ViewMatrix, src.viewMatrix, sizeof(float) * 16);
        std::memcpy(dst.ProjectionMatrix, src.projectionMatrix, sizeof(float) * 16);

        // ViewProjection = Projection * View（列主序矩阵乘法）
        // C# 传来的 view/proj 已 Transpose，Diligent 读为列主序。
        // 列主序下 P * V = 先 V 后 P，与 GLM 的 cameraComp.ProjectionMatrix * viewMat 一致。
        const float* P = src.projectionMatrix;
        const float* V = src.viewMatrix;
        float* VP = dst.ViewProjectionMatrix;
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    sum += P[k * 4 + row] * V[col * 4 + k];
                }
                VP[col * 4 + row] = sum;
            }
        }

        dst.OrthoWidth  = src.orthoWidth;
        dst.OrthoHeight = src.orthoHeight;
        dst.Near = src.nearPlane;
        dst.Far  = src.farPlane;
    }

    /// 校验命令缓冲区
    bool ValidateCommandBuffer(const void* data, std::uint32_t dataSize)
    {
        if (!data || dataSize < sizeof(NNRenderCommandBufferHeader))
        {
            std::cerr << "[ViewportSurface] RenderCommands: 缓冲区无效或过小" << std::endl;
            return false;
        }

        auto* header = static_cast<const NNRenderCommandBufferHeader*>(data);
        if (header->magic != NN_RENDER_COMMAND_BUFFER_MAGIC)
        {
            std::cerr << "[ViewportSurface] RenderCommands: 魔数不匹配 (期望 0x"
                      << std::hex << NN_RENDER_COMMAND_BUFFER_MAGIC
                      << "，实际 0x" << header->magic << std::dec << ")" << std::endl;
            return false;
        }

        if (header->totalBytes != dataSize)
        {
            std::cerr << "[ViewportSurface] RenderCommands: totalBytes 不匹配 (header="
                      << header->totalBytes << "，实际=" << dataSize << ")" << std::endl;
            return false;
        }

        return true;
    }

    std::uint8_t NN_ENGINE_ABI_STDCALL rt_viewportSurface_renderViewportCommands(
        std::uint64_t surfaceId,
        const void* commands,
        std::uint32_t commandsSize)
    {
        // 1. 校验参数
        if (surfaceId == 0 || !commands || commandsSize == 0)
            return 0;

        if (!ValidateCommandBuffer(commands, commandsSize))
            return 0;

        // 确保 RmlUI 渲染器已初始化（惰性，由 ViewportRenderRuntimeApi 管理生命周期）
        EnsureViewportRenderInitialized();

        // 2. 获取 Surface 和 SwapChain
        std::lock_guard<std::mutex> lock(g_SurfaceMutex);
        auto* surface = FindSurface(surfaceId);
        if (!surface || !surface->IsCreated())
            return 0;

        auto* swapChain = surface->GetSwapChain();
        if (!swapChain)
            return 0;

        // 3. 确保 Renderer2D 已初始化
        if (!EnsureCommandsRenderer())
            return 0;

        // 4. 解析命令
        auto* bufferHeader = static_cast<const NNRenderCommandBufferHeader*>(commands);
        const auto* cmdPtr = static_cast<const std::uint8_t*>(commands) + sizeof(NNRenderCommandBufferHeader);
        const auto* cmdEnd = static_cast<const std::uint8_t*>(commands) + commandsSize;

        // 调试日志：dump buffer 头部和前几条命令
        static int s_cmdLogCount = 0;
        if (s_cmdLogCount < 3)
        {
            auto* raw = static_cast<const std::uint8_t*>(commands);
            std::cout << "[RenderCommands] Buffer: magic=0x" << std::hex << bufferHeader->magic
                      << " cmdCount=" << std::dec << bufferHeader->commandCount
                      << " totalBytes=" << bufferHeader->totalBytes
                      << " dataSize=" << commandsSize << std::endl;

            // dump 全部字节（十六进制）
            for (std::uint32_t b = 0; b + 15 < commandsSize; b += 16)
            {
                std::cout << "[RenderCommands] [" << std::dec << b << "] ";
                for (int j = 0; j < 16; j++)
                {
                    std::cout << std::hex << std::setfill('0') << std::setw(2)
                              << static_cast<int>(raw[b + j]) << " ";
                }
                std::cout << std::dec << std::endl;
            }
            s_cmdLogCount++;
        }

        // 暂存解析结果
        NN::Runtime::Renderer2D::CameraData camera{};
        bool hasCamera = false;
        bool clearColorEnabled = false;
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<NN::Runtime::Renderer2D::SpriteDrawCommand> spriteCommands;
        std::vector<NN::Runtime::RmlUI::RmlDrawItem> rmlDrawItems;

        for (std::uint32_t i = 0; i < bufferHeader->commandCount; ++i)
        {
            // 边界检查
            if (cmdPtr + sizeof(NNRenderCommandHeader) > cmdEnd)
            {
                std::cerr << "[ViewportSurface] RenderCommands: 命令 " << i << " 越界" << std::endl;
                return 0;
            }

            auto* cmdHeader = reinterpret_cast<const NNRenderCommandHeader*>(cmdPtr);
            auto offset = static_cast<std::uint32_t>(cmdPtr - static_cast<const std::uint8_t*>(commands));

            // 直接读原始字节验证
            auto b0 = static_cast<int>(cmdPtr[0]);
            auto b1 = static_cast<int>(cmdPtr[1]);
            auto b2 = static_cast<int>(cmdPtr[2]);
            auto b3 = static_cast<int>(cmdPtr[3]);
            auto b4 = static_cast<int>(cmdPtr[4]);
            auto b5 = static_cast<int>(cmdPtr[5]);
            auto b6 = static_cast<int>(cmdPtr[6]);
            auto b7 = static_cast<int>(cmdPtr[7]);

            //std::cout << "[RenderCommands] Cmd[" << i << "]: offset=" << std::dec << offset
            //          << " raw=[" << std::hex << std::setfill('0')
            //          << std::setw(2) << b0 << " " << std::setw(2) << b1 << " "
            //          << std::setw(2) << b2 << " " << std::setw(2) << b3 << " "
            //          << std::setw(2) << b4 << " " << std::setw(2) << b5 << " "
            //          << std::setw(2) << b6 << " " << std::setw(2) << b7
            //          << "] type=0x" << cmdHeader->type
            //          << " size=" << std::dec << cmdHeader->size << std::endl;

            if (cmdPtr + cmdHeader->size > cmdEnd)
            {
                auto cmdOffset = static_cast<std::uint32_t>(cmdPtr - static_cast<const std::uint8_t*>(commands));
                auto remainBytes = static_cast<std::uint32_t>(cmdEnd - cmdPtr);
                std::cerr << "[ViewportSurface] RenderCommands: 命令 " << i
                          << " 数据越界 (type=0x" << std::hex << cmdHeader->type
                          << " size=" << std::dec << cmdHeader->size
                          << " offset=" << cmdOffset
                          << " remain=" << remainBytes
                          << " cmdEnd-cmdPtr=" << (cmdEnd - cmdPtr)
                          << " commandsSize=" << commandsSize << ")" << std::endl;
                return 0;
            }
			//std::cout << NN_RML_DOCUMENT_ENTRY_SIZE  << std::endl;
            switch (cmdHeader->type)
            {
                case NN_RENDER_COMMAND_SET_CAMERA:
                {
                    if (cmdHeader->size < NN_SET_CAMERA_TOTAL_SIZE)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: SetCamera size 不足" << std::endl;
                        return 0;
                    }
                    auto* data = reinterpret_cast<const NNSetCameraData*>(cmdPtr + sizeof(NNRenderCommandHeader));
                    ConvertToCameraData(*data, camera);
                    hasCamera = true;
                    break;
                }

                case NN_RENDER_COMMAND_SET_RENDER_PASS_STATE:
                {
                    if (cmdHeader->size < NN_SET_RENDER_PASS_STATE_TOTAL_SIZE)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: SetRenderPassState size 不足" << std::endl;
                        return 0;
                    }
                    auto* data = reinterpret_cast<const NNRenderPassStateData*>(
                        cmdPtr + sizeof(NNRenderCommandHeader));
                    clearColorEnabled = (data->flags & NN_RENDER_PASS_FLAG_CLEAR_COLOR) != 0;
                    std::memcpy(clearColor, data->clearColor, sizeof(float) * 4);
                    break;
                }

                case NN_RENDER_COMMAND_DRAW_SPRITE_BATCH:
                {
                    if (cmdHeader->size < NN_DRAW_SPRITE_BATCH_HEADER_SIZE)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: DrawSpriteBatch size 不足" << std::endl;
                        return 0;
                    }
                    auto* batchData = reinterpret_cast<const NNDrawSpriteBatchData*>(
                        cmdPtr + sizeof(NNRenderCommandHeader));
                    std::uint32_t spriteCount = batchData->spriteCount;

                    // 校验精灵数据大小
                    std::uint32_t expectedSize = NN_DRAW_SPRITE_BATCH_TOTAL_SIZE(spriteCount);
                    if (cmdHeader->size < expectedSize)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: DrawSpriteBatch 精灵数据不足"
                                  << " (期望=" << expectedSize << " 实际=" << cmdHeader->size << ")" << std::endl;
                        return 0;
                    }

                    // 转换每个 NNSpriteInstance → SpriteDrawCommand
                    auto* sprites = reinterpret_cast<const NNSpriteInstance*>(
                        reinterpret_cast<const std::uint8_t*>(batchData) + sizeof(NNDrawSpriteBatchData));
                    for (std::uint32_t s = 0; s < spriteCount; ++s)
                    {
                        NN::Runtime::Renderer2D::SpriteDrawCommand cmd;
                        ConvertToSpriteDrawCommand(sprites[s], cmd);
                        spriteCommands.push_back(cmd);
                    }
                    break;
                }

                case NN_RENDER_COMMAND_SET_RML_DOCUMENTS:
                {
                    if (cmdHeader->size < NN_SET_RML_DOCUMENTS_HEADER_SIZE)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: SetRmlDocuments size 不足" << std::endl;
                        return 0;
                    }
                    auto* data = reinterpret_cast<const NNRmlDocumentsData*>(
                        cmdPtr + sizeof(NNRenderCommandHeader));
                    std::uint32_t docCount = data->documentCount;

                    // 校验文档数据大小
                    std::uint32_t expectedSize = NN_SET_RML_DOCUMENTS_TOTAL_SIZE(docCount);
                    if (cmdHeader->size < expectedSize)
                    {
                        std::cerr << "[ViewportSurface] RenderCommands: SetRmlDocuments 数据不足"
                                  << " (期望=" << expectedSize << " 实际=" << cmdHeader->size << ")" << std::endl;
                        return 0;
                    }

                    // 转换每个 NNRmlDocumentEntry → RmlDrawItem
                    auto* entries = reinterpret_cast<const NNRmlDocumentEntry*>(
                        reinterpret_cast<const std::uint8_t*>(data) + sizeof(NNRmlDocumentsData));
                    for (std::uint32_t d = 0; d < docCount; ++d)
                    {
                        const auto& entry = entries[d];
                        NN::Runtime::RmlUI::RmlDrawItem item;
                        item.entity = static_cast<std::uint64_t>(entry.entityHandle);
                        item.assetPath = entry.assetPath;  // 直接用路径，不经过 IAssetResolver
                        item.sortOrder = entry.sortOrder;
                        item.viewTarget = static_cast<NN::Runtime::RmlUI::NNRmlUIViewTarget>(entry.viewTarget);
                        item.viewportId = entry.viewportId;
                        rmlDrawItems.push_back(std::move(item));
                    }
                    break;
                }

                default:
                    // 未知命令类型——跳过（向前兼容）
                    H_LOG_WARN("[ViewportSurface] RenderCommands: 未知命令类型 0x{:X}，跳过",
                               cmdHeader->type);
                    break;
            }

            // 移动到下一条命令
            cmdPtr += cmdHeader->size;
        }

        // 5. 获取渲染上下文
        auto* context = surface->GetContext();
        if (!context)
            return 0;

        // 6. 确保 FBO 尺寸与 SwapChain 一致
        auto& swapChainDesc = swapChain->GetDesc();
        std::uint32_t width = swapChainDesc.Width;
        std::uint32_t height = swapChainDesc.Height;

        if (g_CommandsFBO->GetWidth() != width || g_CommandsFBO->GetHeight() != height)
        {
            g_CommandsFBO->Resize(width, height);
        }

        // 7. 设置渲染目标并渲染
        g_CommandsRenderer->SetRenderTarget(
            g_CommandsFBO->GetColorRTV(),
            g_CommandsFBO->GetDepthDSV());

        // 清屏（如果启用了）
        // 注意：Renderer2D 的 BeginScene 可能会清屏，这里暂不单独清
        // 后续可在 Renderer2D 中增加 ClearRenderTarget 方法

        // 如果没有相机命令，使用默认相机
        if (!hasCamera)
        {
            NN::Runtime::Renderer2D::CameraData defaultCamera{};
            camera = defaultCamera;
        }

        g_CommandsRenderer->BeginScene(camera, width, height);
        g_CommandsRenderer->Submit(spriteCommands);
        g_CommandsRenderer->EndScene();

        // 8. RmlUI Overlay Pass（C# 通过 SetRmlDocuments 命令传入文档列表）
        //    管线：Renderer2D (World Pass) → RmlUI (UI Overlay Pass) → CopyTexture → Present
        if (!rmlDrawItems.empty())
        {
            auto* rmlRenderer = GetRmlUIRenderer();
            if (rmlRenderer)
            {
                rmlRenderer->SetViewport(width, height);
                rmlRenderer->Sync(rmlDrawItems);
                rmlRenderer->Update();

                if (g_CommandsFBO && g_CommandsFBO->GetColorRTV())
                {
                    rmlRenderer->RenderOverlayOnScene(
                        rmlDrawItems,
                        NN::Runtime::RmlUI::NNRmlUIViewTarget::Scene,
                        g_CommandsFBO->GetColorRTV(),
                        g_CommandsFBO->GetDepthDSV(),
                        width, height);
                }
            }
        }

        // 9. CopyTexture（FBO → SwapChain back buffer）
        auto* srcTextureView = reinterpret_cast<::Diligent::ITextureView*>(
            g_CommandsFBO->GetColorTextureHandle());
        if (!srcTextureView)
            return 0;

        auto* srcTexture = srcTextureView->GetTexture();
        if (!srcTexture)
            return 0;

        auto* dstRTV = swapChain->GetCurrentBackBufferRTV();
        if (!dstRTV)
            return 0;

        auto* dstTexture = dstRTV->GetTexture();
        if (!dstTexture)
            return 0;

        ::Diligent::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = srcTexture;
        copyAttribs.SrcTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.pDstTexture = dstTexture;
        copyAttribs.DstTextureTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        context->CopyTexture(copyAttribs);

        // 10. Present
        swapChain->Present();

        return 1;
    }

} // anonymous namespace

// ── Builder ──

extern "C" void NNBuildViewportSurfaceRuntimeApi(NNViewportSurfaceAPI* api)
{
    std::cout << "Building ViewportSurface Runtime API..." << std::endl;
    if (api == nullptr)
        return;

    api->CreateSurface   = &rt_viewportSurface_createSurface;
    api->DestroySurface  = &rt_viewportSurface_destroySurface;
    api->MarkResize      = &rt_viewportSurface_markResize;
    api->FlushResizes    = &rt_viewportSurface_flushResizes;
    api->Present         = &rt_viewportSurface_present;
    api->IsSurfaceLost   = &rt_viewportSurface_isSurfaceLost;
    api->RecreateSurface = &rt_viewportSurface_recreateSurface;
    api->RenderViewport  = &rt_viewportSurface_renderViewport;
    api->RenderViewportCommands = &rt_viewportSurface_renderViewportCommands;

    std::cout << "ViewportSurface Runtime API built (v29: +RenderViewportCommands)." << std::endl;
}

// ── Shutdown ──

void ShutdownViewportSurface()
{
    std::lock_guard<std::mutex> lock(g_SurfaceMutex);
    for (auto& [id, surface] : g_Surfaces)
    {
        surface->Destroy();
        delete surface;
    }
    g_Surfaces.clear();
    g_PendingResizes.clear();

    // 清理 RenderCommands 路径的单例
    if (g_CommandsFBO)
    {
        g_CommandsFBO->Shutdown();
        delete g_CommandsFBO;
        g_CommandsFBO = nullptr;
    }
    if (g_CommandsRenderer)
    {
        g_CommandsRenderer->Shutdown();
        delete g_CommandsRenderer;
        g_CommandsRenderer = nullptr;
    }
    g_CommandsRendererInitialized = false;

    std::cout << "[ViewportSurface] 已关闭" << std::endl;
}
