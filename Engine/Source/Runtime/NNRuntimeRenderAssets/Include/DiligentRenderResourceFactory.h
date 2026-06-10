/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNRenderAssetManager.h"
#include "../../NNRuntimeRenderAssets/NNRuntimeRenderAssetsExport.h"

// 前向声明
namespace NN::Runtime::Render { class INNRenderDevice; }

namespace NN::Runtime::Render
{
    /**
     * @brief Diligent 渲染资源工厂。
     * 通过 INNRenderDevice 接口创建 GPU 纹理，后端无关。
     */
    class NN_RUNTIME_RENDER_ASSETS_API DiligentRenderResourceFactory : public IRenderResourceFactory
    {
    public:
        explicit DiligentRenderResourceFactory(INNRenderDevice* device);
        ~DiligentRenderResourceFactory() override = default;

        // IRenderResourceFactory 接口
        std::unique_ptr<NNTextureResource> CreateTexture(
            uint32_t width, uint32_t height,
            NNTextureFormat format,
            const uint8_t* pixels, size_t pixelSize) override;

        bool UpdateTexturePixels(
            NNTextureResource* resource,
            const uint8_t* pixels, size_t pixelSize) override;

    private:
        INNRenderDevice* m_Device = nullptr;  // 非拥有
    };

} // namespace NN::Runtime::Render
