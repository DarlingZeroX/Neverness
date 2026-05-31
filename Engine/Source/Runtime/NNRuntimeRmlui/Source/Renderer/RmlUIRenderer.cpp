/**
 * @file RmlUIRenderer.cpp
 * @brief RmlUI 渲染器实现——持有 Context，管理文档实例，从 DrawList 渲染。
 */

#include "Renderer/RmlUIRenderer.h"
#include "System/NNRmlUISystem.h"

// RmlUI 核心
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Debugger.h>

// 渲染后端
#include "Rml/RmlUi_Renderer_GL3.h"
#include "Rml/RenderInterfaceGL3SDL.h"
#include "Rml/RmlUi_Platform_SDL.h"
#include "Rml/Shell.h"
#include "Rml/ShellFileInterface.h"

// 资产解析
#include "NNRuntimeScene/Include/Assets/IAssetResolver.h"

// GL 状态保存/恢复（RenderToTexture 需要保护 ImGui 的 GL 状态）
#include <NNRuntimeRHI/Include/OpenGL/OpenGL.h>

#include <unordered_set>
#include <iostream>

namespace NN::Runtime::Renderer
{
	RmlUIRenderer::RmlUIRenderer() = default;

	RmlUIRenderer::~RmlUIRenderer()
	{
		Shutdown();
	}

	bool RmlUIRenderer::Initialize(std::uint32_t viewportWidth, std::uint32_t viewportHeight)
	{
		if (m_Initialized)
			return true;

		std::cout << "[RmlUIRenderer] Initialize: 开始 (" << viewportWidth << "x" << viewportHeight << ")" << std::endl;

		m_ViewportWidth = viewportWidth;
		m_ViewportHeight = viewportHeight;

		// 1. 初始化 Shell（VFS 文件接口）
		std::cout << "[RmlUIRenderer] Initialize: Shell::Initialize..." << std::endl;
		if (!Shell::Initialize())
		{
			std::cerr << "[RmlUIRenderer] Shell::Initialize failed" << std::endl;
			return false;
		}

		// 2. 创建渲染后端
		std::cout << "[RmlUIRenderer] Initialize: 创建渲染后端..." << std::endl;
		m_RenderInterface = new RenderInterface_GL3_SDL();
		m_SystemInterface = new SystemInterface_SDL();
		m_RenderInterface->SetViewport((int)viewportWidth, (int)viewportHeight);

		// 3. 设置 RmlUI 全局接口
		std::cout << "[RmlUIRenderer] Initialize: 设置全局接口..." << std::endl;
		Rml::SetSystemInterface(m_SystemInterface);
		Rml::SetRenderInterface(m_RenderInterface);

		// 4. 初始化 RmlUI 核心
		std::cout << "[RmlUIRenderer] Initialize: Rml::Initialise..." << std::endl;
		Rml::Initialise();

		// 5. 创建 Context
		std::cout << "[RmlUIRenderer] Initialize: CreateContext..." << std::endl;
		m_Context = Rml::CreateContext("scene_ui",
			Rml::Vector2i((int)viewportWidth, (int)viewportHeight));
		if (!m_Context)
		{
			std::cerr << "[RmlUIRenderer] Rml::CreateContext failed" << std::endl;
			Rml::Shutdown();
			Shell::Shutdown();
			return false;
		}

		// 6. 加载默认字体
		std::cout << "[RmlUIRenderer] Initialize: LoadFonts..." << std::endl;
		Shell::LoadFonts();

		m_Initialized = true;
		std::cout << "[RmlUIRenderer] Initialize: 完成" << std::endl;
		return true;
	}

	void RmlUIRenderer::Shutdown()
	{
		if (!m_Initialized)
			return;

		// 关闭所有文档
		for (auto& [entity, runtime] : m_Documents)
		{
			if (runtime.doc)
				runtime.doc->Close();
		}
		m_Documents.clear();

		// 销毁 Context
		if (m_Context)
		{
			Rml::RemoveContext(m_Context->GetName());
			m_Context = nullptr;
		}

		// 关闭 RmlUI
		Rml::Shutdown();

		// 关闭 Shell
		Shell::Shutdown();

		// 销毁后端
		delete m_RenderInterface;
		m_RenderInterface = nullptr;
		delete m_SystemInterface;
		m_SystemInterface = nullptr;

		m_Initialized = false;
	}

	void RmlUIRenderer::SetViewport(std::uint32_t width, std::uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		if (m_RenderInterface)
			m_RenderInterface->SetViewport((int)width, (int)height);

		if (m_Context)
			m_Context->SetDimensions(Rml::Vector2i((int)width, (int)height));
	}

	void RmlUIRenderer::Sync(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList)
	{
		if (!m_Initialized)
		{
			std::cerr << "[RmlUIRenderer] Sync: not initialized" << std::endl;
			return;
		}

		std::cout << "[RmlUIRenderer] Sync: drawList size = " << drawList.size()
			<< ", m_Documents size = " << m_Documents.size() << std::endl;

		// 收集 active entity 集合
		std::unordered_set<NNEntity> activeEntities;
		for (const auto& item : drawList)
			activeEntities.insert(item.entity);

		// 卸载不在 list 中的文档
		for (auto it = m_Documents.begin(); it != m_Documents.end(); )
		{
			if (activeEntities.find(it->first) == activeEntities.end())
			{
				std::cout << "[RmlUIRenderer] Sync: unloading document" << std::endl;
				UnloadDocument(it->first);
				it = m_Documents.erase(it);
			}
			else
				++it;
		}

		// 加载新文档
		for (const auto& item : drawList)
		{
			if (m_Documents.find(item.entity) != m_Documents.end())
				continue;

			std::cout << "[RmlUIRenderer] Sync: loading document for entity" << std::endl;
			auto* doc = LoadDocument(item.assetGuid);
			if (doc)
			{
				// 设置文档尺寸为 context 大小，使 CSS 100% 生效
				doc->SetProperty(Rml::PropertyId::Width,
					Rml::Property((float)m_ViewportWidth, Rml::Unit::PX));
				doc->SetProperty(Rml::PropertyId::Height,
					Rml::Property((float)m_ViewportHeight, Rml::Unit::PX));
				doc->Show();
				RmlDocumentRuntime runtime;
				runtime.doc = doc;
				runtime.state = RmlDocState::Ready;
				runtime.assetGuid = item.assetGuid;
				m_Documents[item.entity] = runtime;
			}
			else
			{
				RmlDocumentRuntime runtime;
				runtime.doc = nullptr;
				runtime.state = RmlDocState::Failed;
				runtime.assetGuid = item.assetGuid;
				m_Documents[item.entity] = runtime;
			}
		}

		std::cout << "[RmlUIRenderer] Sync: done" << std::endl;
	}

	void RmlUIRenderer::Update()
	{
		if (!m_Initialized || !m_Context) return;
		m_Context->Update();
	}

	void RmlUIRenderer::Render(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		Scene::NNRmlUIViewTarget viewTarget)
	{
		if (!m_Initialized || !m_Context) return;

		// 按 ViewTarget 控制文档可见性
		for (const auto& item : drawList)
		{
			auto it = m_Documents.find(item.entity);
			if (it == m_Documents.end() || it->second.state != RmlDocState::Ready)
				continue;

			bool shouldShow = (item.viewTarget == Scene::NNRmlUIViewTarget::Both ||
			                   item.viewTarget == viewTarget);

			if (shouldShow && !it->second.doc->IsVisible())
				it->second.doc->Show();
			else if (!shouldShow && it->second.doc->IsVisible())
				it->second.doc->Hide();
		}

		// 渲染所有可见文档（Context::Render 渲染所有 Show 状态的文档）
		m_RenderInterface->BeginFrame();
		m_Context->Render();
		m_RenderInterface->EndFrame();
	}

	std::uint32_t RmlUIRenderer::RenderToTexture(
		const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		Scene::NNRmlUIViewTarget viewTarget)
	{
		if (!m_Initialized || !m_Context || !m_RenderInterface)
			return 0;

		// 按 ViewTarget 控制文档可见性
		for (const auto& item : drawList)
		{
			auto it = m_Documents.find(item.entity);
			if (it == m_Documents.end() || it->second.state != RmlDocState::Ready)
				continue;

			bool shouldShow = (item.viewTarget == Scene::NNRmlUIViewTarget::Both ||
			                   item.viewTarget == viewTarget);

			if (shouldShow && !it->second.doc->IsVisible())
				it->second.doc->Show();
			else if (!shouldShow && it->second.doc->IsVisible())
				it->second.doc->Hide();
		}

		// ── 保存 ImGui 依赖的 GL 状态（RmlUI 会修改这些状态）──
		GLint prevProgram = 0, prevVAO = 0, prevVBO = 0, prevFBO = 0;
		GLint prevTexture = 0, prevActiveTex = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVBO);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
		glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);

		// ── RmlUI 渲染到内部 FBO ──
		// 检查文档状态（仅首帧打印）
		static bool s_loggedDocState = false;
		if (!s_loggedDocState)
		{
			for (const auto& [entity, runtime] : m_Documents)
			{
				if (runtime.doc)
				{
					std::cout << "[RmlUIRenderer] doc visible=" << runtime.doc->IsVisible()
						<< " state=" << static_cast<int>(runtime.state)
						<< " url=" << runtime.doc->GetSourceURL() << std::endl;
				}
			}
			std::cout << "[RmlUIRenderer] context docs: " << m_Context->GetNumDocuments() << std::endl;
			s_loggedDocState = true;
		}

		// 每帧更新所有文档尺寸（匹配当前 viewport 大小）
		for (auto& [entity, runtime] : m_Documents)
		{
			if (runtime.doc && runtime.state == RmlDocState::Ready)
			{
				runtime.doc->SetProperty(Rml::PropertyId::Width,
					Rml::Property((float)m_ViewportWidth, Rml::Unit::PX));
				runtime.doc->SetProperty(Rml::PropertyId::Height,
					Rml::Property((float)m_ViewportHeight, Rml::Unit::PX));
			}
		}

		m_Context->Update();
		m_RenderInterface->BeginFrame();
		m_Context->Render();
		m_RenderInterface->EndFrameNoBlit();

		// ── 恢复 ImGui 依赖的 GL 状态 ──
		glUseProgram(prevProgram);
		glBindVertexArray(prevVAO);
		glBindBuffer(GL_ARRAY_BUFFER, prevVBO);
		glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
		glActiveTexture(prevActiveTex);
		glBindTexture(GL_TEXTURE_2D, prevTexture);

		// 获取渲染结果纹理 ID
		const auto& fb = m_RenderInterface->GetRenderResult();
		return static_cast<std::uint32_t>(fb.color_tex_buffer);
	}

	void RmlUIRenderer::ProcessInput(
		std::uint32_t type,
		std::int32_t mouseX, std::int32_t mouseY,
		std::int32_t wheelX, std::int32_t wheelY,
		std::uint32_t button,
		std::uint32_t keyCode, std::uint32_t keyMod)
	{
		if (!m_Initialized || !m_Context) return;

		switch (type)
		{
		case 0: // MouseMove
			m_Context->ProcessMouseMove(mouseX, mouseY, 0);
			break;
		case 1: // MouseButtonDown
			m_Context->ProcessMouseButtonDown((int)button, 0);
			break;
		case 2: // MouseButtonUp
			m_Context->ProcessMouseButtonUp((int)button, 0);
			break;
		case 3: // MouseWheel
			m_Context->ProcessMouseWheel((float)wheelY, 0);
			break;
		case 4: // KeyDown
			m_Context->ProcessKeyDown((Rml::Input::KeyIdentifier)keyCode, 0);
			break;
		case 5: // KeyUp
			m_Context->ProcessKeyUp((Rml::Input::KeyIdentifier)keyCode, 0);
			break;
		}
	}

	void RmlUIRenderer::SetAssetResolver(NN::Runtime::Scene::IAssetResolver* resolver)
	{
		m_AssetResolver = resolver;
	}

	Rml::ElementDocument* RmlUIRenderer::LoadDocument(NNGuid assetGuid)
	{
		if (!m_Context)
		{
			std::cerr << "[RmlUIRenderer] LoadDocument: m_Context is null" << std::endl;
			return nullptr;
		}
		if (!m_AssetResolver)
		{
			std::cerr << "[RmlUIRenderer] LoadDocument: m_AssetResolver is null" << std::endl;
			return nullptr;
		}

		// 通过 IAssetResolver 解析路径
		char path[512];
		path[0] = '\0';
		if (!m_AssetResolver->Resolve(assetGuid, path, sizeof(path)))
		{
			std::cerr << "[RmlUIRenderer] AssetResolver failed for GUID ("
				<< assetGuid.high << ":" << assetGuid.low << ")" << std::endl;
			return nullptr;
		}

		// 检查路径是否为空
		if (path[0] == '\0')
		{
			std::cerr << "[RmlUIRenderer] AssetResolver returned empty path for GUID ("
				<< assetGuid.high << ":" << assetGuid.low << ")" << std::endl;
			return nullptr;
		}

		std::cout << "[RmlUIRenderer] LoadDocument: resolved path = " << path << std::endl;

		// 加载文档
		auto* doc = m_Context->LoadDocument(path);
		if (!doc)
		{
			std::cerr << "[RmlUIRenderer] LoadDocument failed: " << path << std::endl;
		}
		else
		{
			std::cout << "[RmlUIRenderer] LoadDocument success: " << path << std::endl;
		}

		return doc;
	}

	void RmlUIRenderer::UnloadDocument(NNEntity entity)
	{
		auto it = m_Documents.find(entity);
		if (it != m_Documents.end() && it->second.doc)
		{
			it->second.doc->Close();
		}
	}

} // namespace NN::Runtime::Renderer
