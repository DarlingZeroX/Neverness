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
#include "../VGUIConfig.h"
//#include "../Scene/Scene.h"
#include "NNRuntimeCore/Include/Core/Window.h"
#include "NNRuntimeCore/Interface/EngineInterface.h"
#include "NNRuntimeCore/Include/Core/Viewport.h"
#include "UIDocument.h"
#include <string>
#include <RmlUi/Core.h>

namespace NN::Runtime
{
	class VG_UI_API UISystem: public IUISystem,public NN::Core::SDL3::Layer
	{
	public:
		UISystem();
		~UISystem() override;

		static UISystem* Get();

		int Initialize(NN::Core::SDL3::OpenGLWindow* window, Viewport* viewport);

		Ref<RmlUIDocument> LoadUIDocument(const String& path);
		bool ShowUIDocument(RmlUIDocument* doc);
		void ReloadUIDocument(Ref<RmlUIDocument>& doc);
		void ReloadAllUIDocument();
		void CloseAllDocuments();
		Ref<RmlUIDocument> FindDocumentByElementDocument(Rml::ElementDocument* document);
		 
		Ref<RmlUIDocument> OnScriptOpenDocument(Rml::ElementDocument* document);
		// Source/UI/Lua/Document.cpp 122行使用了这里
		void OnScriptCloseDocument(const Rml::ElementDocument* document);

		// Returns a pointer to the custom system interface which should be provided to RmlUi.
		Rml::SystemInterface* GetSystemInterface() const;
		// Returns a pointer to the custom render interface which should be provided to RmlUi.
		Rml::RenderInterface* GetRenderInterface() const;

		void Render();
		void BeginFrame();
		void RenderContext();
		void EndFrame();

		unsigned int GetUIRenderResult();

		void OnUpdate();
		void* GetContext() override { return m_pContext; }
		bool ProcessContextEvent(Rml::Context* context, const SDL_Event& evt);
		bool ProcessContextEventViewport(Rml::Context* context, const SDL_Event& evt);
		int ProcessEvent(const SDL_Event& event) override;
	private:
		int Initialize(NN::Core::SDL3::OpenGLWindow* window);
		bool InitializeUISystem(NN::Core::SDL3::OpenGLWindow* window);
		bool InitializeRuntimeEnvironment();
	private:
		Viewport* m_Viewport = nullptr;
		Rml::Context* m_pContext;

		NN::Core::SDL3::OpenGLWindow* m_Window;

		Rml::SystemInterface* m_SystemInterface;
		Rml::RenderInterface* m_RenderInterface;

		std::function<int(Rml::Context*, const SDL_Event&)> m_ProcessContextEventFunction;
		std::vector<Ref<RmlUIDocument>> m_Documents;
		//std::vector<Rml::ElementDocument*> m_NativeDocument;

		std::vector<std::function<void()>> m_CloseCallbacks;
	};


}


