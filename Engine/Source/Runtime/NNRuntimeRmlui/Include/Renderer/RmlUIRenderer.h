#pragma once

/**
 * @file RmlUIRenderer.h
 * @brief RmlUI 渲染器——持有 Rml::Context，管理文档实例，从 DrawList 渲染。
 *
 * 职责：
 * - 管理 Rml::Context / RenderInterface / SystemInterface 生命周期
 * - 根据 RmlDrawList 加载/卸载文档（Sync 阶段）
 * - 更新 Context（Update 阶段）
 * - 根据 ViewTarget 过滤并渲染文档（Render 阶段）
 * - 处理输入事件
 *
 * 渲染后端：RmlDiligent（Diligent Engine）
 *
 * 已移除：
 * - NNRuntimeScene 依赖（移至 Legacy）
 * - IAssetResolver 接口（简化为直接路径加载）
 */

#include "Header.h"

namespace NN::Runtime::Renderer
{
	/**
	 * @brief RmlUI 渲染器——持有 Context，管理文档实例。
	 */
	class VG_UI_API RmlUIRenderer
	{
	public:
		RmlUIRenderer();
		~RmlUIRenderer();

		RmlUIRenderer(const RmlUIRenderer&) = delete;
		RmlUIRenderer& operator=(const RmlUIRenderer&) = delete;

		/// @brief 初始化渲染器。
		/// @param device Diligent 渲染设备
		/// @param viewportWidth 视口宽度
		/// @param viewportHeight 视口高度
		bool Initialize(Render::INNRenderDevice* device,
		                std::uint32_t viewportWidth, std::uint32_t viewportHeight);
		void Shutdown();

		void SetViewport(std::uint32_t width, std::uint32_t height);

		/// @brief 同步 DrawList（Update 阶段调用）。
		void Sync(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList);

		/// @brief 更新 Context（Update 阶段调用）。
		void Update();

		/// @brief 纯渲染（Render 阶段调用）。
		void Render(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		            NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget);

		/// @brief 渲染到内部 RenderTarget 并返回纹理句柄。
		/// 适用于 Editor 子渲染器场景（ImGui 帧内调用）。
		/// @return RmlUI 渲染结果的纹理句柄（uint64_t，0 = 无内容或失败）。
		std::uint64_t RenderToTexture(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		                              NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget);

		/// @brief 在已有 Scene RT 上叠加渲染 RmlUI（alpha 混合）。
		void RenderOverlayOnScene(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		                          NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget,
		                          void* sceneRTV, void* sceneDSV,
		                          std::uint32_t width, std::uint32_t height);

		/// @brief 处理输入事件。
		void ProcessInput(std::uint32_t type,
		                  std::int32_t mouseX, std::int32_t mouseY,
		                  std::int32_t wheelX, std::int32_t wheelY,
		                  std::uint32_t button,
		                  std::uint32_t keyCode, std::uint32_t keyMod);

		/// @brief 设置资产路径解析器。
		void SetAssetResolver(NN::Runtime::RmlUI::IRmlUIAssetResolver* resolver);

		/// @brief 热重载：按 VFS 路径关闭并重新加载指定文档。
		///
		/// 查找 SourceURL 匹配的文档，关闭后从 m_Documents 移除。
		/// 下次 Sync() 时会自动重新加载（路径仍在 DrawList 中）。
		void ReloadDocumentByPath(const std::string& vfsPath);

		/// @brief 热重载：关闭并重新加载所有文档。
		///
		/// 关闭全部文档，清空 m_Documents。
		/// 下次 Sync() 时会自动重新加载。
		void ReloadAllDocuments();

	private:
		Rml::ElementDocument* LoadDocument(NNGuid assetGuid);
		Rml::ElementDocument* LoadDocumentByPath(const std::string& assetPath);
		void UnloadDocument(NN::Runtime::RmlUI::NNEntity entity);

		// Diligent 后端
		Render::INNRenderDevice* m_Device = nullptr;  // 观察指针
		RmlDiligent::RmlDiligentRenderInterface* m_RenderInterface = nullptr;
		Render::INNRenderTarget* m_OffscreenRT = nullptr;  // 离屏渲染目标

		// 平台后端（保留 Runtime 版本，支持 VFS）
		SystemInterface_SDL* m_SystemInterface = nullptr;
		UIFileInterfaceVFS* m_FileInterface = nullptr;

		Rml::Context* m_Context = nullptr;
		NN::Runtime::RmlUI::IRmlUIAssetResolver* m_AssetResolver = nullptr;

		// NNEntity（uint64_t）→ 文档运行时实例
		std::unordered_map<NN::Runtime::RmlUI::NNEntity, RmlDocumentRuntime,
		                   NN::Runtime::RmlUI::NNEntityHash> m_Documents;

		std::uint32_t m_ViewportWidth = 0;
		std::uint32_t m_ViewportHeight = 0;
		bool m_Initialized = false;
	};

} // namespace NN::Runtime::Renderer
