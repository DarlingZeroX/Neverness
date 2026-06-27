/**
* @file RmlIRenderer.cpp
* @brief RmlUI 渲染器实现——RmlDiligent 后端（Diligent Engine）。
*
* 替换原 GL3 后端，使用 RmlDiligentRenderInterface。
* 保留 Runtime 的 SystemInterface_SDL 和 UIFileInterfaceVFS（VFS 支持）。
*
* 已移除：
* - NNRuntimeScene 依赖（移至 Legacy）
* - IAssetResolver 接口（简化为 IRmlUIAssetResolver）
*/

#include "Renderer/RmlRenderer.h"
#include "System/NNRmlUISystem.h"

// RmlUI 核心
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Debugger.h>

// RmlDiligent 渲染后端
#include "RmlDiligentRenderInterface.h"

// Runtime 平台后端（保留 VFS 支持）
#include "Rml/RmlUi_Platform_SDL.h"
#include "Rml/Shell.h"
#include "Rml/ShellFileInterface.h"

// NNRuntimeRender 接口
#include <Device/INNRenderDevice.h>
#include <RenderTarget/INNRenderTarget.h>

// NNDiligent 内部（用于获取 Diligent 原始对象）
#include "NNDiligentConfig.h"
#include "Device/NNDiligentDevice.h"
#include "Resources/NNDiligentRenderTarget.h"

#include <unordered_set>
#include <iostream>

using namespace Diligent;

namespace NN::Runtime::Renderer
{
	RmlRenderer::RmlRenderer() = default;

	RmlRenderer::~RmlRenderer()
	{
		Shutdown();
	}

	bool RmlRenderer::Initialize(
		Render::INNRenderDevice* device,
		std::uint32_t viewportWidth, std::uint32_t viewportHeight)
	{
		if (m_Initialized)
			return true;

		if (!device)
		{
			std::cerr << "[RmlIRenderer] Initialize: device is null" << std::endl;
			return false;
		}

		m_Device = device;
		m_ViewportWidth = viewportWidth;
		m_ViewportHeight = viewportHeight;

		std::cout << "[RmlIRenderer] Initialize: 开始 ("
			<< viewportWidth << "x" << viewportHeight << ")" << std::endl;

		// 1. 初始化 Shell（VFS 文件接口）
		std::cout << "[RmlIRenderer] Initialize: Shell::Initialize..." << std::endl;
		if (!Shell::Initialize())
		{
			std::cerr << "[RmlIRenderer] Shell::Initialize failed" << std::endl;
			return false;
		}

		// 2. 获取 Diligent 原始对象
		auto* dilDev = static_cast<NNDiligent::NNDiligentDevice*>(device);
		auto* diliDevice  = dilDev->GetDiligentDevice();
		auto* diliContext = dilDev->GetDiligentContext();
		auto* diliSwapChain = dilDev->GetDiligentSwapChain();

		if (!diliDevice || !diliContext)
		{
			std::cerr << "[RmlIRenderer] Diligent device/context is null" << std::endl;
			Shell::Shutdown();
			return false;
		}

		// 3. 创建 RmlDiligent 渲染后端
		std::cout << "[RmlIRenderer] Initialize: 创建 RmlDiligent 渲染后端..." << std::endl;
		m_RenderInterface = new RmlDiligent::RmlDiligentRenderInterface();
		m_RenderInterface->Initialize(diliDevice, diliContext, diliSwapChain);
		m_RenderInterface->SetProjectionMatrix((int)viewportWidth, (int)viewportHeight);

		// 4. 创建平台后端（Runtime 版本，支持 VFS）
		m_SystemInterface = new SystemInterface_SDL();

		// 5. 设置 RmlUI 全局接口
		std::cout << "[RmlIRenderer] Initialize: 设置全局接口..." << std::endl;
		Rml::SetSystemInterface(m_SystemInterface);
		Rml::SetRenderInterface(m_RenderInterface);

		// 6. 初始化 RmlUI 核心
		std::cout << "[RmlIRenderer] Initialize: Rml::Initialise..." << std::endl;
		
		
		//Rml::Initialise();

		// 7. 创建 Context
		//std::cout << "[RmlIRenderer] Initialize: CreateContext..." << std::endl;
		//m_Context = Rml::CreateContext("scene_ui",
		//	Rml::Vector2i((int)viewportWidth, (int)viewportHeight));
		//if (!m_Context)
		//{
		//	std::cerr << "[RmlIRenderer] Rml::CreateContext failed" << std::endl;
		//	Rml::Shutdown();
		//	Shell::Shutdown();
		//	return false;
		//}

		// 8. 加载默认字体
		std::cout << "[RmlIRenderer] Initialize: LoadFonts..." << std::endl;
		Shell::LoadFonts();

		// 9. 创建离屏渲染目标
		Render::NNRenderTargetDesc rtDesc{};
		rtDesc.Width = viewportWidth;
		rtDesc.Height = viewportHeight;
		rtDesc.ColorFormat = Render::NNPixelFormat::RGBA8_UNORM;
		rtDesc.DepthFormat = Render::NNPixelFormat::D24_UNORM_S8_UINT;
		auto rt = device->CreateRenderTarget(rtDesc);
		if (rt)
			m_OffscreenRT = rt.Detach();

		m_Initialized = true;
		std::cout << "[RmlIRenderer] Initialize: 完成" << std::endl;
		return true;
	}

	void RmlRenderer::Shutdown()
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
		//if (m_Context)
		//{
		//	Rml::RemoveContext(m_Context->GetName());
		//	m_Context = nullptr;
		//}

		// 关闭 RmlUI
		Rml::Shutdown();

		// 关闭 Shell
		Shell::Shutdown();

		// 销毁离屏渲染目标
		if (m_OffscreenRT)
		{
			m_OffscreenRT->Release();
			m_OffscreenRT = nullptr;
		}

		// 销毁后端
		delete m_RenderInterface;
		m_RenderInterface = nullptr;
		delete m_SystemInterface;
		m_SystemInterface = nullptr;

		m_Device = nullptr;
		m_Initialized = false;
	}

	void RmlRenderer::SetViewport(std::uint32_t width, std::uint32_t height)
	{
		// 尺寸未变化时跳过（避免每帧重建离屏 RT 导致内存泄漏 + 描述符堆耗尽）
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		if (m_RenderInterface)
			m_RenderInterface->SetProjectionMatrix((int)width, (int)height);

		if (m_Context)
			m_Context->SetDimensions(Rml::Vector2i((int)width, (int)height));

		// 重建离屏渲染目标（仅在尺寸变化时）
		if (m_OffscreenRT)
		{
			m_OffscreenRT->Release();
			m_OffscreenRT = nullptr;
		}
		if (m_Device && width > 0 && height > 0)
		{
			Render::NNRenderTargetDesc rtDesc{};
			rtDesc.Width = width;
			rtDesc.Height = height;
			rtDesc.ColorFormat = Render::NNPixelFormat::RGBA8_UNORM;
			rtDesc.DepthFormat = Render::NNPixelFormat::D24_UNORM_S8_UINT;
			auto rt = m_Device->CreateRenderTarget(rtDesc);
			if (rt)
				m_OffscreenRT = rt.Detach();
		}
	}

	void RmlRenderer::Sync(const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList)
	{
		if (!m_Initialized)
		{
			std::cerr << "[RmlIRenderer] Sync: not initialized" << std::endl;
			return;
		}

		using namespace NN::Runtime::RmlUI;

		// 收集 active entity 集合
		std::unordered_set<NNEntity> activeEntities;
		for (const auto& item : drawList)
			activeEntities.insert(item.entity);

		// 三路 Diff：删除不在 list 中的文档
		for (auto it = m_Documents.begin(); it != m_Documents.end(); )
		{
			if (activeEntities.find(it->first) == activeEntities.end())
			{
				std::cout << "[RmlIRenderer] Sync: unloading document entity=" << it->first << std::endl;
				UnloadDocument(it->first);
				it = m_Documents.erase(it);
			}
			else
				++it;
		}

		// 三路 Diff：加载新文档（优先用 assetPath，回退到 assetGuid → IAssetResolver）
		for (const auto& item : drawList)
		{
			if (m_Documents.find(item.entity) != m_Documents.end())
				continue;

			std::cout << "[RmlIRenderer] Sync: loading document entity=" << item.entity
				<< " path=" << (item.assetPath.empty() ? "(from guid)" : item.assetPath) << std::endl;

			Rml::ElementDocument* doc = nullptr;
			if (!item.assetPath.empty())
			{
				// RenderCommands 路径：直接用路径加载
				doc = LoadDocumentByPath(item.assetPath);
			}

			if (doc)
			{
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
	}

	void RmlRenderer::Update()
	{
		if (!m_Initialized || !m_Context) return;
		m_Context->Update();
	}

	void RmlRenderer::RenderOverlayOnScene(
		const std::vector<NN::Runtime::RmlUI::RmlDrawItem>& drawList,
		NN::Runtime::RmlUI::NNRmlUIViewTarget viewTarget,
		void* sceneRTV, void* sceneDSV,
		std::uint32_t width, std::uint32_t height)
	{
		using namespace NN::Runtime::RmlUI;

		if (!m_Initialized || !m_Context || !m_RenderInterface || !sceneRTV)
			return;

		// 按 ViewTarget 控制文档可见性
		for (const auto& item : drawList)
		{
			auto it = m_Documents.find(item.entity);
			if (it == m_Documents.end() || it->second.state != RmlDocState::Ready)
				continue;

			bool shouldShow = (item.viewTarget == NNRmlUIViewTarget::Both ||
				item.viewTarget == viewTarget);

			if (shouldShow && !it->second.doc->IsVisible())
				it->second.doc->Show();
			else if (!shouldShow && it->second.doc->IsVisible())
				it->second.doc->Hide();
		}

		// 每帧更新所有文档尺寸
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

		// 在 Scene RT 上叠加渲染 RmlUI（alpha 混合，不清除 Scene 内容）
		auto* rtv = static_cast<Diligent::ITextureView*>(sceneRTV);
		auto* dsv = static_cast<Diligent::ITextureView*>(sceneDSV);

		m_Context->Update();
		m_RenderInterface->CompositeOnTop(rtv, dsv, static_cast<int>(width), static_cast<int>(height));
		m_Context->Render();
		m_RenderInterface->EndOffscreenFrame();
	}

	void RmlRenderer::SetContext(Rml::Context* ctx)
	{
		m_Context = ctx;
	}

	Rml::ElementDocument* RmlRenderer::LoadDocumentByPath(const std::string& assetPath)
	{
		if (!m_Context)
		{
			std::cerr << "[RmlIRenderer] LoadDocumentByPath: m_Context is null" << std::endl;
			return nullptr;
		}

		if (assetPath.empty())
		{
			std::cerr << "[RmlIRenderer] LoadDocumentByPath: assetPath is empty" << std::endl;
			return nullptr;
		}

		std::cout << "[RmlIRenderer] LoadDocumentByPath: " << assetPath << std::endl;

		auto* doc = m_Context->LoadDocument(assetPath.c_str());
		if (!doc)
		{
			std::cerr << "[RmlIRenderer] LoadDocumentByPath failed: " << assetPath << std::endl;
		}
		else
		{
			std::cout << "[RmlIRenderer] LoadDocumentByPath success: " << assetPath << std::endl;
		}

		return doc;
	}

	void RmlRenderer::UnloadDocument(NN::Runtime::RmlUI::NNEntity entity)
	{
		auto it = m_Documents.find(entity);
		if (it != m_Documents.end() && it->second.doc)
		{
			it->second.doc->Close();
		}
	}

} // namespace NN::Runtime::Renderer
