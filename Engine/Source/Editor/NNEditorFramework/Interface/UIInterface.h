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
#include <string>
#include <NNRuntimeImGui/Include/ImGuiEx/ImPanelInterface.h>

namespace VisionGal::Editor {

	struct IPanel 
	{
		virtual ~IPanel() = default;

		virtual void OnUpdate(float delta) {};
		virtual void OnFixedUpdate() {};
		virtual bool IsAsync() { return false; }
		virtual void OnGUI() {};
	};

	struct IEditorPanel: public IPanel
	{
		virtual std::string GetWindowFullName() = 0;
		virtual std::string GetWindowName() = 0;
		virtual void OpenWindow(bool open) = 0;
		virtual bool IsWindowOpened() = 0;
	};

	struct ISidebarComponent: public IPanel
	{
		~ISidebarComponent() override = default;

		virtual void Toggle() = 0;
		virtual void OnSideBarUI() = 0;
	};

	template<typename T>
	struct IComponentUIRenderer
	{
		virtual ~IComponentUIRenderer() = default;

		virtual void OnGUI(T* obj) = 0;
		virtual const std::string GetBindType() const = 0;
	};
}