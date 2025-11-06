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
#include "CrossPlatformDefinitions.h"
#include "HMouseBase.h"

#include "../../Core/HCore.h"
#include <queue>
#include <optional>

namespace Horizon
{
	class HMouseContext;

	class CROSS_PLATFORM_API HMouseButtonEvent
	{
	public:
		enum class Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			MPress,
			MRelease,
			WheelUp,
			WheelDown,
			Move,
			Enter,
			Leave,
		};
	private:
		Type type;
		bool leftIsPressed;
		bool rightIsPressed;
		int x;
		int y;
	public:
		HMouseButtonEvent(Type type, const HMouseContext& parent) noexcept;

		std::pair<int, int> GetPos() const noexcept;
		int GetPosX() const noexcept;
		int GetPosY() const noexcept;
		bool LeftIsPressed() const noexcept;
		bool RightIsPressed() const noexcept;
	};

	class CROSS_PLATFORM_API HMouseContext
		:
		public HMouseContextWriterInterface
	{
		friend class Window;

	public:
		struct StateTracker
		{
			HMouseButtonState leftButton;
			HMouseButtonState middleButton;
			HMouseButtonState rightButton;

#pragma prefast(suppress: 26495, "Reset() performs the initialization")
			StateTracker() noexcept;

			void __cdecl Update(const HMouseState& state) noexcept;

			void __cdecl OnLeftButton(bool leftPressed) noexcept;
			void __cdecl OnMiddleButton(bool middlePressed) noexcept;
			void __cdecl OnRightButton(bool rightPressed) noexcept;

			void __cdecl Reset() noexcept;

		private:
			HMouseState __cdecl GetLastState() const noexcept;

			HMouseState lastState;
		};
	public:
		HMouseContext() = default;
		HMouseContext(const HMouseContext&) = delete;
		HMouseContext& operator=(const HMouseContext&) = delete;
		~HMouseContext() override = default;
	public:
		void OnMouseMove(int x, int y) noexcept override;
		void OnMouseLeave() noexcept override;
		void OnMouseEnter() noexcept override;
		void OnRawDelta(float dx, float dy) noexcept override;

		void OnLeftPressed() noexcept override;
		void OnLeftReleased() noexcept override;
		void OnMiddlePressed() noexcept override;
		void OnMiddleReleased() noexcept override;
		void OnRightPressed() noexcept override;
		void OnRightReleased() noexcept override;

		void OnWheelUp(int x, int y) noexcept override;
		void OnWheelDown(int x, int y) noexcept override;
		void OnWheelDelta(int x, int y, int delta) noexcept override;
	public:
		HMousePos GetPos() const noexcept;
		HMouseRawDelta ReadRawDelta() noexcept;
		bool ReadRawDelta(HMouseRawDelta& delta) noexcept;

		bool IsInWindow() const noexcept;
		bool LeftIsPressed() const noexcept;
		bool RightIsPressed() const noexcept;

		std::optional<HMouseButtonEvent> Read() noexcept;
		bool IsEmpty() const noexcept;

		void Flush() noexcept;
		void EnableRaw() noexcept;
		void DisableRaw() noexcept;
		bool RawEnabled() const noexcept;
		void ClearRawBuffer() noexcept;

		void TrimBuffer() noexcept;
		void TrimRawInputBuffer() noexcept;

		void FrameUpdate();
		void FixedUpdate();

		const StateTracker& GetStateTracker() const noexcept;

		HMouseState state;
	private:
		static constexpr unsigned int bufferSize = 16u;

		StateTracker stateTracker;

		bool isInWindow = false;
		bool rawEnabled = false;

		std::queue<HMouseButtonEvent> buffer;
		std::queue<HMouseRawDelta> rawDeltaBuffer;
	};

	struct CROSS_PLATFORM_API HMouse
	{
		HMouse();
		HMouse(const HMouse&) = delete;
		HMouse& operator=(const HMouse&) = delete;
		~HMouse() = default;

		std::shared_ptr<HMouseInterface>  GetAttachedMouse();
		H_NODISCARD bool VaildAttachedMouse();
		void MouseAttached(std::shared_ptr<HMouseInterface> mouse);

		// Sets mouse mode (defaults to absolute)
		bool SetRelativeMode(HRelativeMouseMode mode);

		// Check mouse state
		H_NODISCARD bool IsMouseButtonDown(HMouseButton bt) const;
		H_NODISCARD bool IsMouseButtonUp(HMouseButton bt) const;
		H_NODISCARD bool IsMouseButtonHeld(HMouseButton bt) const;

		H_NODISCARD HMouseRawDelta GetRawDelta() const noexcept;
		H_NODISCARD HMousePos GetMousePosition() const noexcept;

		H_NODISCARD HRelativeMouseMode GetRelativeMode() const noexcept;

		void FrameUpdate();
		void FixedUpdate();

		// Retrieve the current context of the mouse
		H_NODISCARD const HMouseContext& GetContext() const noexcept;

		HPROPERTY_READWRITE(bool) Visible;

		HPROPERTY_READWRITE(HRelativeMouseMode) RelativeMode;
	private:
		std::weak_ptr<HMouseInterface> m_wAttachedMouse;
		HMouseContext m_MouseContext;
	};
}