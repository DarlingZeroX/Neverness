#pragma once
#include "../EngineConfig.h"
#include "../Scene/Scene.h"
#include "../Core/Window.h"
#include "../Interface/EngineInterface.h"
#include "../Core/Viewport.h"
#include "../Resource/UIDocument.h"
#include <string>
#include <RmlUi/Core.h>

namespace VisionGal
{
	class VG_ENGINE_API UISystem: public IUISystem,public Horizon::SDL3::Layer
	{
	public:
		UISystem();
		~UISystem() override;

		static UISystem* Get();

		int Initialize(Horizon::SDL3::OpenGLWindow* window, Viewport* viewport);
		bool LoadDocument(std::string doc);

		Ref<RmlUIDocument> LoadUIDocument(const String& path);
		bool ShowUIDocument(const Ref<RmlUIDocument>& doc);
		bool ShowUIDocument(const RmlUIDocument* doc);
		void ReloadUIDocument(Ref<RmlUIDocument>& doc);
		void CloseAllDocuments();
		 
		void OnScriptOpenDocument(Rml::ElementDocument* document);
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
		int Initialize(Horizon::SDL3::OpenGLWindow* window);
		bool InitializeUISystem(Horizon::SDL3::OpenGLWindow* window);
		bool InitializeRuntimeEnvironment();
	private:
		Viewport* m_Viewport = nullptr;
		Rml::Context* m_pContext;

		Horizon::SDL3::OpenGLWindow* m_Window;

		Rml::SystemInterface* m_SystemInterface;
		Rml::RenderInterface* m_RenderInterface;

		std::function<int(Rml::Context*, const SDL_Event&)> m_ProcessContextEventFunction;
		std::vector<Ref<RmlUIDocument>> m_Documents;
		std::vector<Rml::ElementDocument*> m_NativeDocument;
	};


}


