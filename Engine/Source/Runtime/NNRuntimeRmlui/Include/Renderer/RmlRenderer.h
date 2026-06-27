#pragma once
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

        /// @brief 更新 Context（Update 阶段调用）。
        void Update();

		/// @brief 在已有 Scene RT 上叠加渲染 RmlUI（alpha 混合）。
		void RenderOverlayOnScene(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
			NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget,
			void* sceneRTV, void* sceneDSV,
			std::uint32_t width, std::uint32_t height);

		void SetContext(Rml::Context* ctx);

    private:
        // Diligent 后端
        Render::INNRenderDevice* m_Device = nullptr;
        Render::INNRenderTarget* m_OffscreenRT = nullptr;

        Rml::Context* m_Context = nullptr;

        std::uint32_t m_ViewportWidth = 0;
        std::uint32_t m_ViewportHeight = 0;
        bool m_Initialized = false;
    };

} // namespace NN::Runtime::Renderer
