/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "Engine/ImGuiLayer.h"

#include <SDL3/SDL.h>
#include <NNPlatformCore/Include/SDL3/SDL3Window.h>

// ImGui 核心（NNRuntimeImGui 编译，v1.92.3）
#include <imgui.h>
#include "imgui_internal.h"

// Diligent ImGui 后端（类接口）
#include <ImGuiImplDiligent.hpp>
#include <ImGuiImplSDL3.hpp>

// Diligent 设备访问
#include <Device/INNRenderDevice.h>
#include <Device/INNSwapChain.h>
#include "Device/NNDiligentDevice.h"

using namespace Diligent;

namespace NN::Runtime
{
	struct ImguiDiligentLayer::Impl
	{
		std::unique_ptr<ImGuiImplSDL3> ImGuiBackend;
		IDeviceContext* Context = nullptr;
		ISwapChain* SwapChain = nullptr;
	};

	ImguiDiligentLayer::ImguiDiligentLayer(NN::Core::SDL3::Window* window, Render::INNRenderDevice* device)
		: m_Impl(std::make_unique<Impl>())
	{
		if (!window || !device)
			return;

		auto* dilDev = static_cast<NNDiligent::NNDiligentDevice*>(device);
		auto* diliDevice = dilDev->GetDiligentDevice();
		m_Impl->Context = dilDev->GetDiligentContext();
		m_Impl->SwapChain = dilDev->GetDiligentSwapChain();

		if (!diliDevice || !m_Impl->Context || !m_Impl->SwapChain)
			return;

		// 获取交换链格式
		const auto& SCDesc = m_Impl->SwapChain->GetDesc();

		// 创建 Diligent ImGui 后端
		ImGuiDiligentCreateInfo CI{diliDevice, SCDesc};
		m_Impl->ImGuiBackend = ImGuiImplSDL3::Create(CI, window->GetSDLWindow());
	}

	ImguiDiligentLayer::~ImguiDiligentLayer()
	{
		ImGuiShutdown();
	}

	void ImguiDiligentLayer::BeginFrame()
	{
		if (!m_Impl || !m_Impl->ImGuiBackend || !m_Impl->SwapChain)
			return;

		const auto& SCDesc = m_Impl->SwapChain->GetDesc();
		m_Impl->ImGuiBackend->NewFrame(
			SCDesc.Width,
			SCDesc.Height,
			SCDesc.PreTransform
		);
		// 诊断：打印 GImGui 地址和 FrameCount
		//{
		//	auto* ctx = ImGui::GetCurrentContext();
		//	auto* g = ctx;
		//	printf("[BeginFrame] ctx=%p FrameCount=%d FrameCountEnded=%d IMGUI_VERSION=%s\n",
		//		   ctx, g ? (int)g->FrameCount : -1, g ? (int)g->FrameCountEnded : -1, IMGUI_VERSION);
		//}
		////ImGui::NewFrame();
	}

	void ImguiDiligentLayer::EndFrame()
	{
		if (!m_Impl || !m_Impl->ImGuiBackend || !m_Impl->Context || !m_Impl->SwapChain)
			return;

		// Diligent ImGui 后端不会自动绑定渲染目标，
		// 必须在 Render 之前把 SwapChain 的 RTV/DSV 设好，否则 Vulkan 后端断言失败
		auto* pRTV = m_Impl->SwapChain->GetCurrentBackBufferRTV();
		auto* pDSV = m_Impl->SwapChain->GetDepthBufferDSV();
		m_Impl->Context->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		m_Impl->ImGuiBackend->Render(m_Impl->Context);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImguiDiligentLayer::ImGuiInit()
	{
		// 初始化在构造函数中完成
	}

	void ImguiDiligentLayer::ImGuiShutdown()
	{
		if (m_Impl)
		{
			m_Impl->ImGuiBackend.reset();
		}
		ImGui::DestroyContext();
	}

	void ImguiDiligentLayer::ImGuiRender()
	{
	}

}
