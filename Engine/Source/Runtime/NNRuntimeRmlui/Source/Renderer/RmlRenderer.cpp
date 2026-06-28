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
	struct RmlSystem
	{
		static RmlSystem* Get()
		{
			return s_Instance;
		}

		static void CreateOrGet(Render::INNRenderDevice* device)
		{
			if (s_Instance == nullptr)
				s_Instance = new RmlSystem(device);
		}

		RmlSystem(Render::INNRenderDevice* device)
		{
			Initialize(device);
		}

		bool Initialize(Render::INNRenderDevice* device)
		{
			int viewportWidth = 1280;
			int viewportHeight = 720;

			if (!device)
			{
				std::cerr << "[RmlSystem] Initialize: device is null" << std::endl;
				return false;
			}

			m_Device = device;

			std::cout << "[RmlSystem] Initialize: 开始 ("
				<< viewportWidth << "x" << viewportHeight << ")" << std::endl;

			// 1. 初始化 Shell（VFS 文件接口）
			std::cout << "[RmlSystem] Initialize: Shell::Initialize..." << std::endl;
			if (!Shell::Initialize())
			{
				std::cerr << "[RmlSystem] Shell::Initialize failed" << std::endl;
				return false;
			}
			
			// 2. 获取 Diligent 原始对象
			auto* dilDev = static_cast<NNDiligent::NNDiligentDevice*>(device);
			auto* diliDevice  = dilDev->GetDiligentDevice();
			auto* diliContext = dilDev->GetDiligentContext();
			auto* diliSwapChain = dilDev->GetDiligentSwapChain();

			if (!diliDevice || !diliContext)
			{
				std::cerr << "[RmlSystem] Diligent device/context is null" << std::endl;
				Shell::Shutdown();
				return false;
			}

			// 3. 创建 RmlDiligent 渲染后端
			std::cout << "[RmlSystem] Initialize: 创建 RmlDiligent 渲染后端..." << std::endl;
			m_RenderInterface = new RmlDiligent::RmlDiligentRenderInterface();
			m_RenderInterface->Initialize(diliDevice, diliContext, diliSwapChain);
			m_RenderInterface->SetProjectionMatrix((int)viewportWidth, (int)viewportHeight);

			// 4. 创建平台后端（Runtime 版本，支持 VFS）
			m_SystemInterface = new SystemInterface_SDL();

			// 5. 设置 RmlUI 全局接口
			std::cout << "[RmlSystem] Initialize: 设置全局接口..." << std::endl;
			Rml::SetSystemInterface(m_SystemInterface);
			Rml::SetRenderInterface(m_RenderInterface);

			// 6. 初始化 RmlUI 核心
			std::cout << "[RmlSystem] Initialize: Rml::Initialise..." << std::endl;
			Rml::Initialise();

			// 8. 加载默认字体
			std::cout << "[RmlIRenderer] Initialize: LoadFonts..." << std::endl;
			Shell::LoadFonts();

			return true;
		}

		~RmlSystem()
		{
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
		}

		// Diligent 后端
		Render::INNRenderDevice* m_Device = nullptr;
		RmlDiligent::RmlDiligentRenderInterface* m_RenderInterface = nullptr;
		Render::INNRenderTarget* m_OffscreenRT = nullptr;

		// 平台后端
		SystemInterface_SDL* m_SystemInterface = nullptr;
		UIFileInterfaceVFS* m_FileInterface = nullptr;
	private:
		static RmlSystem* s_Instance;
	};

	// 静态成员定义
	RmlSystem* RmlSystem::s_Instance = nullptr;

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

		m_Device = device;
		RmlSystem::CreateOrGet(device);

		m_ViewportWidth = viewportWidth;
		m_ViewportHeight = viewportHeight;

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

		// 销毁离屏渲染目标
		if (m_OffscreenRT)
		{
			m_OffscreenRT->Release();
			m_OffscreenRT = nullptr;
		}
		m_Initialized = false;
	}

	void RmlRenderer::SetViewport(std::uint32_t width, std::uint32_t height)
	{
		// 尺寸未变化时跳过（避免每帧重建离屏 RT 导致内存泄漏 + 描述符堆耗尽）
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		if (RmlSystem::Get()->m_RenderInterface)
			RmlSystem::Get()->m_RenderInterface->SetProjectionMatrix((int)width, (int)height);

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

		if (!m_Initialized || !m_Context || !RmlSystem::Get()->m_RenderInterface || !sceneRTV)
			return;

		// 在 Scene RT 上叠加渲染 RmlUI（alpha 混合，不清除 Scene 内容）
		auto* rtv = static_cast<Diligent::ITextureView*>(sceneRTV);
		auto* dsv = static_cast<Diligent::ITextureView*>(sceneDSV);

		m_Context->Update();
		RmlSystem::Get()->m_RenderInterface->CompositeOnTop(rtv, dsv, static_cast<int>(width), static_cast<int>(height));
		m_Context->Render();
		RmlSystem::Get()->m_RenderInterface->EndOffscreenFrame();
	}

	void RmlRenderer::SetContext(Rml::Context* ctx)
	{
		m_Context = ctx;
	}

} // namespace NN::Runtime::Renderer
