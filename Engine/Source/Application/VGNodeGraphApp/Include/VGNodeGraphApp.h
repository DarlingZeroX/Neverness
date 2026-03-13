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
#include <VGEngine/Include/Core/Window.h>
#include <VGEngine/Include/Game/GameEngine.h>
#include <VGEngine/Include/Engine/ImGuiLayer.h>
#include <VGEngine/Include/Interface/EngineInterface.h>

namespace VisionGal::Editor
{
	class VGNodeGraphApp : public IEngineApplication
	{
	public:
		VGNodeGraphApp() = default;
		VGNodeGraphApp(const VGNodeGraphApp&) = delete;
		VGNodeGraphApp& operator=(const VGNodeGraphApp&) = delete;
		VGNodeGraphApp(VGNodeGraphApp&&) noexcept = default;
		VGNodeGraphApp& operator=(VGNodeGraphApp&&) noexcept = default;
		~VGNodeGraphApp() override;

		void Initialize();
		void AddApplicationLayer(IEngineApplicationLayer* layer) override;
		void OnApplicationUpdate(float deltaTime) override;
		int ProcessEvent(const SDL_Event& event) override;
		void MakeCurrentRenderContext() override;

		void InitializeEditorPanels();
	private:
		void InitializeEditorUI();
		void AddImguiLayer();
	private:
		void OnRender();
		void OnUpdate(float delta);
		void OnFixedUpdate();
		void OnGUI();
	private:
		Ref<VGWindow> m_ApplicationWindow;
		Scope<ImguiOpengl3Layer> m_ImguiOpengl3Layer;
	};


}
