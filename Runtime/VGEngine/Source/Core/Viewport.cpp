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

#include "Core/Viewport.h"

namespace VisionGal
{
	Viewport::Viewport(const float2& viewport_size)
		:m_State(viewport_size)
	{
	}

	void Viewport::SetViewportPosition(float2 pos)
	{
		m_State.ViewportPosition = pos;
	}

	void Viewport::SetViewportSize(float2 size)
	{
		if (m_State.ViewportSize != size)
		{

			m_State.ViewportSize = size;

			OnViewportSizeChanged(size);
		}
	}

	void Viewport::SetViewportHoverState(bool hovering)
	{
		m_State.ViewportHovering = hovering;
	}

	float Viewport::Width()
	{
		return m_State.ViewportSize.x;
	}

	float Viewport::Height()
	{
		return m_State.ViewportSize.y;
	}

	const ViewportState& Viewport::GetState() const
	{
		return m_State;
	}

	void Viewport::MouseMoved(float2 MousePosition)
	{
		m_State.MouseScreenPosition = MousePosition;

		ViewportEvent evt;
		evt.MousePosition = float2(MousePosition);
		evt.Type = ViewportEventType::MouseMove;

		m_FrameEvents.emplace_back(std::move(evt));
	}

	void Viewport::Focus()
	{
		ViewportEvent evt;
		evt.Type = ViewportEventType::Focused;

		m_FrameEvents.emplace_back(std::move(evt));
	}

	void Viewport::UnFocus()
	{
		ViewportEvent evt;
		evt.Type = ViewportEventType::UnFocused;

		m_FrameEvents.emplace_back(std::move(evt));
	}

	void Viewport::SetViewportTexture(VGFX::ITexture* texture)
	{
		m_State.ViewportTexture = texture;
	}

	VGFX::ITexture* Viewport::GetViewportTexture()
	{
		return m_State.ViewportTexture;
	}

	void Viewport::MouseClick(float2 MousePosition)
	{
		m_State.MouseScreenPosition = MousePosition;
		m_State.IsClicked = true;

		ViewportEvent evt;
		evt.MousePosition = float2(MousePosition);
		evt.Type = ViewportEventType::MouseClicked;

		m_FrameEvents.emplace_back(std::move(evt));
	}

	void Viewport::MouseDragging(float2 MousePosition)
	{
		m_State.MouseScreenPosition = MousePosition;
		m_State.IsDragging = true;

		ViewportEvent evt;
		evt.MousePosition = float2(MousePosition);
		evt.Type = ViewportEventType::MouseDragging;

		m_FrameEvents.emplace_back(std::move(evt));
	}

	void Viewport::SetMouseHoveredData(const MouseHoveredData& data)
	{
		m_State.MouseHoveredData = data;
	}

	std::unordered_set<ICamera*>& Viewport::GetCameras()
	{
		return m_Cameras;
	}

	void Viewport::AttachCamera(ICamera* camera)
	{
		m_Cameras.insert(camera);
	}

	bool Viewport::RemoveCamera(ICamera* camera)
	{
		m_Cameras.erase(camera);
		return true;
	}

	void Viewport::AttachWindow(Horizon::IWindow* window)
	{
		m_Window = window;
	}

	Horizon::IWindow* Viewport::GetWindow()
	{
		return m_Window;
	}

	void Viewport::EnableInput(bool enable)
	{
		m_IsEnableInput = enable;
	}

	bool Viewport::IsEnableInput() const
	{
		return m_IsEnableInput;
	}

	void Viewport::SetWindowID(WindowID id)
	{
		m_WindowID = id;
	}

	WindowID Viewport::GetWindowID() const
	{
		return m_WindowID;
	}

	void Viewport::FrameUpdate()
	{
		for (int i = 0; i < m_FrameEvents.size(); ++i)
		{
			OnViewportEvent.Invoke(m_FrameEvents[i]);
		}

		m_FrameEvents.clear();

		m_State.IsResizing = false;
		m_State.IsClicked = false;
		m_State.IsDragging = false;
	}

	void Viewport::OnViewportSizeChanged(float2 size)
	{
		for (auto& camera: m_Cameras)
		{
			camera->OnViewportSizeChange(size.x, size.y);
		}

		m_State.IsResizing = true;
		ViewportEvent evt;
		evt.NewViewportSize = float2(size);
		evt.Type = ViewportEventType::SizeChanged;

		m_FrameEvents.emplace_back(std::move(evt));
	}
}


