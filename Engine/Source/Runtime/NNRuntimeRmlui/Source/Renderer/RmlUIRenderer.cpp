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

		m_ViewportWidth = viewportWidth;
		m_ViewportHeight = viewportHeight;

		// 1. 初始化 Shell（VFS 文件接口）
		if (!Shell::Initialize())
		{
			std::cerr << "[RmlUIRenderer] Shell::Initialize failed" << std::endl;
			return false;
		}

		// 2. 创建渲染后端
		m_RenderInterface = new RenderInterface_GL3_SDL();
		m_SystemInterface = new SystemInterface_SDL();
		m_RenderInterface->SetViewport((int)viewportWidth, (int)viewportHeight);

		// 3. 设置 RmlUI 全局接口
		Rml::SetSystemInterface(m_SystemInterface);
		Rml::SetRenderInterface(m_RenderInterface);

		// 4. 初始化 RmlUI 核心
		Rml::Initialise();

		// 5. 创建 Context
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
		Shell::LoadFonts();

		m_Initialized = true;
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
		if (!m_Initialized) return;

		// 收集 active entity 集合
		std::unordered_set<NNEntity> activeEntities;
		for (const auto& item : drawList)
			activeEntities.insert(item.entity);

		// 卸载不在 list 中的文档
		for (auto it = m_Documents.begin(); it != m_Documents.end(); )
		{
			if (activeEntities.find(it->first) == activeEntities.end())
			{
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

			auto* doc = LoadDocument(item.assetGuid);
			if (doc)
			{
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
		if (!m_Context || !m_AssetResolver)
			return nullptr;

		// 通过 IAssetResolver 解析路径
		char path[512];
		if (!m_AssetResolver->Resolve(assetGuid, path, sizeof(path)))
		{
			std::cerr << "[RmlUIRenderer] AssetResolver failed for GUID" << std::endl;
			return nullptr;
		}

		// 加载文档
		auto* doc = m_Context->LoadDocument(path);
		if (!doc)
		{
			std::cerr << "[RmlUIRenderer] LoadDocument failed: " << path << std::endl;
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
