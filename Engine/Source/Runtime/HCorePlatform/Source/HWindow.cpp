/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "HWindow.h"
#include "HCore/Interface/HVector.h"

namespace Horizon
{
	HWindow::HWindow(int width, int height)
	{
		//auto* window = new SDL3::Window(width, height);
		IWindow* window = nullptr;

		window->AddWindowEventListener([this](const Events::HWindowEvent& evt)
			{
				OnWindowEvent.Invoke(evt);
			});

		m_pWindow = std::unique_ptr<IWindow>(window);
	}

	std::string HWindow::GetTitle() const noexcept
	{
		return m_pWindow->GetTitle();
	}

	void HWindow::SetTitle(const std::string& title)
	{
		m_pWindow->SetTitle(title);
	}

	IWindow* HWindow::GetWrapWindow() noexcept
	{
		return m_pWindow.get();
	}

	void* HWindow::GetNativeWindow() noexcept
	{
		return m_pWindow->GetNativeWindow();
	}

	EWindow HWindow::GetWindowType() const noexcept
	{
		return m_pWindow->GetWindowType();
	}

	uint32 HWindow::GetWidth() const noexcept
	{
		return m_pWindow->WindowWidth();
	}

	uint32 HWindow::GetHeight() const noexcept
	{
		return m_pWindow->WindowHeight();
	}

	void HWindow::SetPosition(uint32 x, uint32 y)
	{
		return m_pWindow->SetWindowPos(x, y);
	}

	uint2 HWindow::GetPosition() const
	{
		return m_pWindow->GetWindowPos();
	}

	void HWindow::MinimizeWindow()
	{
		return m_pWindow->MinimizeWindow();
	}

	void HWindow::MaximizeWindow()
	{
		return m_pWindow->MaximizeWindow();
	}

	void HWindow::RestoreWindow()
	{
		return m_pWindow->RestoreWindow();
	}

	void HWindow::SetFullscreen(bool fullscreen)
	{
		return m_pWindow->SetFullscreen(fullscreen);
	}

	void HWindow::Close()
	{
		return m_pWindow->Close();
	}

	int HWindow::ProcessMessages() noexcept
	{
		return m_pWindow->ProcessMessages();
	}

	void HWindow::Clear() noexcept
	{
		return m_pWindow->Clear();
	}
}