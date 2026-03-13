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
#include "../../HCorePlatformConfig.h"
#include "HKeyboardBase.h"
#include <HCore/Include/HConfig.h>
#include <bitset>
#include <queue>
#include <optional>

namespace Horizon
{
	class HKeyboardEvent
	{
	public:
		enum class Type
		{
			Press,
			Release,
		};
	private:
		Type type;
		unsigned char code;
	public:
		HKeyboardEvent(Type type, unsigned char code) noexcept;

		bool IsPress() const noexcept;

		bool IsRelease() const noexcept;

		unsigned char GetCode() const noexcept;
	};

	struct H_CORE_PLATFORM_API HKBState
	{
	private:
		static constexpr unsigned int nKeys = 256u;
	public:
		std::bitset<nKeys> released;
		std::bitset<nKeys> pressed;
		std::bitset<nKeys> held;

		//bool __cdecl IsKeyHeld(HKeycode key) const noexcept;
		void Reset();
	};

	class H_CORE_PLATFORM_API HKeyboardContext
		:
		public HKeyboardContextWriterInterface
	{
	public:
		HKeyboardContext() = default;
		HKeyboardContext(const HKeyboardContext&) = delete;
		HKeyboardContext& operator=(const HKeyboardContext&) = delete;
		~HKeyboardContext() override = default;
	public:
		void OnKeyPressed(unsigned char keycode) noexcept;
		void OnKeyReleased(unsigned char keycode) noexcept;
		void OnChar(char character) noexcept;
	public:
		// key event stuff
		std::optional<HKeyboardEvent> ReadKey() noexcept;
		bool KeyIsEmpty() const noexcept;
		void FlushKey() noexcept;

		// char event stuff
		std::optional<char> ReadChar() noexcept;
		bool CharIsEmpty() const noexcept;
		void FlushChar() noexcept;
		void Flush() noexcept;

		// autorepeat control
		void EnableAutorepeat() noexcept;
		void DisableAutorepeat() noexcept;
		bool AutorepeatIsEnabled() const noexcept;

		void ClearState() noexcept;

		static void TrimBuffer(std::queue<HKeyboardEvent>& buffer) noexcept;
		static void TrimBuffer(std::queue<char>& buffer) noexcept;

		void Update();
	private:
		static constexpr unsigned int nKeys = 256u;
		static constexpr unsigned int bufferSize = 16u;

		bool autorepeatEnabled = false;
		std::queue<HKeyboardEvent> keybuffer;
		std::queue<char> charbuffer;

		std::bitset<nKeys> keyReleased;
		std::bitset<nKeys> keyPressed;

	public:
		HKBState states;
		//std::bitset<nKeys>& GetKeyState() { return keystates; }
	};

	//class CROSS_PLATFORM_API HKeyboardContext
	//{
	//public:
	//	HKeyboardContext();
	//	virtual ~HKeyboardContext() = default;
	//
	//	HKeyboardContext(HKeyboardContext&& moveFrom) noexcept = default;
	//	HKeyboardContext& operator= (HKeyboardContext&& moveFrom) noexcept = default;
	//
	//	HKeyboardContext(const HKeyboardContext&) = default;
	//	HKeyboardContext& operator=(const HKeyboardContext&) = default;
	//
	//	// Retrieve the current state of the keyboard
	//	virtual const HKBState& __cdecl State() const;
	//	virtual const HKBStateTracker& __cdecl StateTracker() const;
	//
	//	// Reset the keyboard state
	//	virtual void __cdecl Reset();
	//
	//	virtual void Update();
	//
	//	HKBState& state;
	//	HKBStateTracker tracker;
	//	HKBInputBuffer inputBuffer;
	//};

	struct H_CORE_PLATFORM_API HKeyboard
	{
		~HKeyboard() = default;

		std::shared_ptr<HKeyboardInterface>  GetAttachedKeyboard();
		bool VaildAttachedKeyboard();
		void KeyboardAttached(std::shared_ptr<HKeyboardInterface> mouse);

		// Retrieve the current state of the mouse
		//const HKBState& State() const;

		void Update();

		bool IsKeyDown(HKeycode key) const;
		bool IsKeyUp(HKeycode key) const;
		bool IsKeyHeld(HKeycode key) const;

		HKeyboardContext& GetContext() const noexcept;
	private:
		std::weak_ptr<HKeyboardInterface> m_wAttachedKeyboard;
		HKeyboardContext m_KeyboardContext;
	};
}
