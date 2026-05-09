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
#include <VGCore/Include/Core/Window.h>
#include <VGEngine/Include/Game/GameEngine.h>
#include <VGEngine/Include/Engine/ImGuiLayer.h>
#include <VGCore/Interface/EngineInterface.h>

namespace VisionGal::Editor
{
	class VGEditorApplication: public IEngineApplication
	{
	public:
		VGEditorApplication() = default;
		VGEditorApplication(const VGEditorApplication&) = delete;
		VGEditorApplication& operator=(const VGEditorApplication&) = delete;
		VGEditorApplication(VGEditorApplication&&) noexcept = default;
		VGEditorApplication& operator=(VGEditorApplication&&) noexcept = default;
		~VGEditorApplication() override;

		void Initialize();
		void AddApplicationLayer(IEngineApplicationLayer* layer) override;
		void OnApplicationUpdate(float deltaTime) override;
		int ProcessEvent(const SDL_Event& event) override;
		void MakeCurrentRenderContext() override;
	private:
		void InitializeEditorUI();
		void AddImguiLayer();
	private:
		void OnRender();
		void OnUpdate(float delta);
		void OnFixedUpdate();
		void OnGUI();
	private:
		Ref<VGWindow> m_EditorWindow;
		Scope<ImguiOpengl3Layer> m_ImguiOpengl3Layer;
		Ref<CoreGameEngine> m_GameEngine;
	};
}
