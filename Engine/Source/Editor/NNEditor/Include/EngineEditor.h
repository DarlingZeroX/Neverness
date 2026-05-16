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
#include <NNRuntimeCore/Include/Core/Window.h>
#include <NNEngineLegacy/Include/Game/GameEngine.h>
#include <NNEngineLegacy/Include/Engine/ImGuiLayer.h>
#include <NNRuntimeCore/Interface/EngineInterface.h>

namespace VisionGal::Editor
{
	class VGEditorApplication: public NN::Runtime::IEngineApplication
	{
	public:
		VGEditorApplication() = default;
		VGEditorApplication(const VGEditorApplication&) = delete;
		VGEditorApplication& operator=(const VGEditorApplication&) = delete;
		VGEditorApplication(VGEditorApplication&&) noexcept = default;
		VGEditorApplication& operator=(VGEditorApplication&&) noexcept = default;
		~VGEditorApplication() override;

		void Initialize();
		void AddApplicationLayer(NN::Runtime::IEngineApplicationLayer* layer) override;
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
		NN::Ref<NN::Runtime::VGWindow> m_EditorWindow;
		NN::Scope<NN::Runtime::ImguiOpengl3Layer> m_ImguiOpengl3Layer;
		NN::Ref<NN::Runtime::CoreGameEngine> m_GameEngine;
	};
}
