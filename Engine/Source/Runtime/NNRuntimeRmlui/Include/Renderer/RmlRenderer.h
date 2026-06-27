#pragma once

/**
 * @file RmlRenderer.h
 * @brief RmlUI 渲染器（新版）——匹配 C# Neverness.Runtime.Rmlui 的 ABI 接口。
 *
 * 从 RmlUIRenderer 复制，用于 C# ABI 调用。
 * 文档管理使用 entity（uint64_t）作为标识，与原始 RmlUIRenderer 一致。
 *
 * 渲染后端：RmlDiligent（Diligent Engine）
 */

#include "Header.h"

namespace NN::Runtime::Renderer
{
    /**
     * @brief RmlUI 渲染器（新版）——持有 Context，管理文档实例。
     *
     * 匹配 C# Neverness.Runtime.Rmlui 的 ABI 接口。
     * 文档管理使用 entity（uint64_t）作为标识。
     */
    class VG_UI_API RmlRenderer
    {
    public:
        RmlRenderer();
        ~RmlRenderer();

        RmlRenderer(const RmlRenderer&) = delete;
        RmlRenderer& operator=(const RmlRenderer&) = delete;

        /// @brief 初始化渲染器。
        bool Initialize(Render::INNRenderDevice* device,
                        std::uint32_t viewportWidth, std::uint32_t viewportHeight);
        void Shutdown();

        void SetViewport(std::uint32_t width, std::uint32_t height);

		/// @brief 同步 DrawList（Update 阶段调用）。
		void Sync(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList);

        /// @brief 更新 Context（Update 阶段调用）。
        void Update();

		/// @brief 在已有 Scene RT 上叠加渲染 RmlUI（alpha 混合）。
		void RenderOverlayOnScene(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
			NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget,
			void* sceneRTV, void* sceneDSV,
			std::uint32_t width, std::uint32_t height);

		void SetContext(Rml::Context* ctx);

    private:
		Rml::ElementDocument* LoadDocumentByPath(const std::string& assetPath);
		void UnloadDocument(NN::Runtime::RmlUI::NNEntity entity);

        // Diligent 后端
        Render::INNRenderDevice* m_Device = nullptr;
        RmlDiligent::RmlDiligentRenderInterface* m_RenderInterface = nullptr;
        Render::INNRenderTarget* m_OffscreenRT = nullptr;

        // 平台后端
        SystemInterface_SDL* m_SystemInterface = nullptr;
        UIFileInterfaceVFS* m_FileInterface = nullptr;

        Rml::Context* m_Context = nullptr;

        // entity → 文档运行时实例
        std::unordered_map<std::uint64_t, RmlDocumentRuntime> m_Documents;

        std::uint32_t m_ViewportWidth = 0;
        std::uint32_t m_ViewportHeight = 0;
        bool m_Initialized = false;
    };

} // namespace NN::Runtime::Renderer
