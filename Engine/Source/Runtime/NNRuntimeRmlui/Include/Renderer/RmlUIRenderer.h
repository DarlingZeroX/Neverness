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
 * 生命周期分离（v4 架构审查修正）：
 * - Sync()  — Update 阶段调用，加载/卸载文档（含磁盘 IO）
 * - Update() — 更新 Context（Layout + 动画）
 * - Render() — 纯渲染，不加载/卸载任何资源
 */

#include "../../VGUIConfig.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

// 包含 NNEntity 和 NNGuid 定义
#include "NNNativeEngineAPI/Include/EngineTypes.h"
#include "NNRuntimeScene/Include/Scene/NNEntity.h"
#include "NNRuntimeScene/Include/Components/NNRmlUIDocumentComponent.h"

// RmlUI 核心头文件
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

// 前向声明渲染后端
class RenderInterface_GL3;
class SystemInterface_SDL;
class UIFileInterfaceVFS;

// 前向声明 IAssetResolver
namespace NN::Runtime::Scene { class IAssetResolver; }

// 包含完整定义（NNEntityHash 需要完整定义用于 unordered_map）
#include "../System/NNRmlUISystem.h"

namespace NN::Runtime::Renderer
{
	/**
	 * @brief 文档运行时状态（简化，无 Loading）。
	 */
	enum class RmlDocState : std::uint8_t
	{
		Ready,      ///< 已加载，可渲染
		Hidden,     ///< 已加载但不可见
		Failed,     ///< 加载失败
	};

	/**
	 * @brief 文档运行时实例（Renderer 内部管理）。
	 */
	struct RmlDocumentRuntime
	{
		Rml::ElementDocument* doc = nullptr;
		RmlDocState state = RmlDocState::Ready;
		NNGuid assetGuid{};
	};

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

		bool Initialize(std::uint32_t viewportWidth, std::uint32_t viewportHeight);
		void Shutdown();

		void SetViewport(std::uint32_t width, std::uint32_t height);

		/// @brief 同步 DrawList（Update 阶段调用）。
		void Sync(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList);

		/// @brief 更新 Context（Update 阶段调用）。
		void Update();

		/// @brief 纯渲染（Render 阶段调用）。
		void Render(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		            NN::Runtime::Scene::NNRmlUIViewTarget viewTarget);

		/// @brief 渲染到内部 FBO 并返回纹理 ID（不 blit 到屏幕）。
		/// 适用于 Editor 子渲染器场景（ImGui 帧内调用）。
		/// @return RmlUI 渲染结果的 OpenGL 纹理 ID（0 = 无内容或失败）。
		std::uint32_t RenderToTexture(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		                              NN::Runtime::Scene::NNRmlUIViewTarget viewTarget);

		/// @brief 处理输入事件。
		void ProcessInput(std::uint32_t type,
		                  std::int32_t mouseX, std::int32_t mouseY,
		                  std::int32_t wheelX, std::int32_t wheelY,
		                  std::uint32_t button,
		                  std::uint32_t keyCode, std::uint32_t keyMod);

		/// @brief 设置资产路径解析器。
		void SetAssetResolver(Scene::IAssetResolver* resolver);

	private:
		Rml::ElementDocument* LoadDocument(NNGuid assetGuid);
		void UnloadDocument(NN::Runtime::Scene::NNEntity entity);

		Rml::Context* m_Context = nullptr;
		RenderInterface_GL3* m_RenderInterface = nullptr;
		SystemInterface_SDL* m_SystemInterface = nullptr;
		UIFileInterfaceVFS* m_FileInterface = nullptr;
		Scene::IAssetResolver* m_AssetResolver = nullptr;

		// NNEntity（带 generation）→ 文档运行时实例
		std::unordered_map<NN::Runtime::Scene::NNEntity, RmlDocumentRuntime,
		                   NN::Runtime::RmlUI::NNEntityHash> m_Documents;

		std::uint32_t m_ViewportWidth = 0;
		std::uint32_t m_ViewportHeight = 0;
		bool m_Initialized = false;
	};

} // namespace NN::Runtime::Renderer
