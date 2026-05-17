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

#pragma once
#include "Events.h"
#include "RuntimeCore.h"
#include "../../Interface/GameInterface.h"
//#include "../Graphics/Interface/Texture.h"
#include <NNRuntimeRHI/Interface/Texture.h>
#include <NNPlatformCore/Include/WindowInterface.h>
#include <NNCore/Include/Event/HEventDelegate.h>

namespace NN::Runtime
{
	struct MouseHoveredData
	{
		bool Valid;
		float ZDepth;
		float3 WorldPosition;
		float3 WorldNormal;
		uint ObjectID;

		MouseHoveredData()
			: Valid(false),
			ZDepth(0),
			WorldPosition(0),
			WorldNormal(0),
			ObjectID(0)
		{
		}
	};

	struct ViewportState
	{
		bool IsMainViewport = false;
		bool ViewportHovering = false;
		bool IsResizing = false;
		bool IsClicked = false;
		bool IsDragging = false;

		float2 ViewportPosition;
		float2 ViewportSize;
		float2 MouseScreenPosition;

		MouseHoveredData MouseHoveredData;

		VGFX::ITexture* ViewportTexture = nullptr;

		ViewportState() = default;
		ViewportState(const float2& viewport_size)
			: 
			ViewportSize(viewport_size),
			ViewportPosition(0.0f)
		{
		}
	};

	class NN_RUNTIME_CORE_API Viewport: public IViewport
	{
	public:
		Viewport() = default;
		Viewport(const float2& viewport_size);
		Viewport(const Viewport&) = delete;
		Viewport& operator=(const Viewport&) = delete;
		Viewport(Viewport&&) noexcept = default;
		Viewport& operator=(Viewport&&) noexcept = default;
		~Viewport() override = default;

		void SetViewportPosition(float2 pos);
		void SetViewportSize(float2 size);
		void SetViewportHoverState(bool hovering);

		float Width();
		float Height();

		const ViewportState& GetState() const;

		void MouseMoved(float2 MousePosition);

		void Focus();
		void UnFocus();

		void SetViewportTexture(VGFX::ITexture* texture);
		VGFX::ITexture* GetViewportTexture();

		void MouseClick(float2 MousePosition);
		void MouseDragging(float2 MousePosition);
		void SetMouseHoveredData(const MouseHoveredData& data);

		std::unordered_set<ICamera*>& GetCameras();
		void AttachCamera(ICamera* camera) override;
		bool RemoveCamera(ICamera* camera) override;

		void AttachWindow(NN::Core::IWindow* window);
		NN::Core::IWindow* GetWindow();

		void EnableInput(bool enable = true);
		bool IsEnableInput() const;

		void SetWindowID(WindowID id);
		WindowID GetWindowID() const;

		NN::Core::HEventDelegate<const ViewportEvent&> OnViewportEvent;

		void FrameUpdate();
	private:
		void OnViewportSizeChanged(float2 size);
	private:
		NN::Core::IWindow* m_Window;
		std::unordered_set<ICamera*> m_Cameras;
		ViewportState m_State;
		std::vector<ViewportEvent> m_FrameEvents;
		bool m_IsEnableInput = true;
		WindowID m_WindowID = 0;
	};
}
