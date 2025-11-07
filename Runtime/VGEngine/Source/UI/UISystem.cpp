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

#include "UI/UISystem.h"
#include <RmlUi/Debugger.h>
#include "Engine/Manager.h"
#include "Galgame/GameLua.h"
#include "UI/Rml/Shell.h"
#include "UI/Rml/RmlUi_Platform_SDL.h"
#include "UI/Rml/RmlUi_Renderer_GL3.h"
#include "UI/Rml/RenderInterfaceGL3SDL.h"
#include <algorithm>

namespace VisionGal
{
	UISystem::UISystem()
	{
	}

	UISystem::~UISystem()
	{
		CloseAllDocuments();
		Rml::Shutdown();
		Shell::Shutdown();
	}

	UISystem* UISystem::Get()
	{
		static UISystem s_UISystem;
		return &s_UISystem;
	}

	int UISystem::Initialize(Horizon::SDL3::OpenGLWindow* window)
	{
		m_Window = window;
		m_Window->AddLayer(this);
		SDL_GL_MakeCurrent(m_Window->GetSDLWindow(), m_Window->GetContext());

		InitializeUISystem(window);
		InitializeRuntimeEnvironment();

		m_ProcessContextEventFunction = [this](Rml::Context* context, const SDL_Event& evt) { return ProcessContextEvent(context, evt); };
		return 0;
	}

	RenderInterface_GL3* GetRenderInterfaceGL(Rml::RenderInterface* interface)
	{
		return  dynamic_cast<RenderInterface_GL3*>(interface);
	}

	int UISystem::Initialize(Horizon::SDL3::OpenGLWindow* window, Viewport* viewport)
	{
		Initialize(window);

		m_Viewport = viewport;
		m_ProcessContextEventFunction = [this](Rml::Context* context, const SDL_Event& evt) { return ProcessContextEventViewport(context, evt); };

		m_Viewport->OnViewportEvent.Subscribe([this](const ViewportEvent& evt)
			{
				if (evt.Type == ViewportEventType::SizeChanged)
				{
					int2 viewportSize = m_Viewport->GetState().ViewportSize;
					Rml::Vector2i dimensions = { viewportSize.x, viewportSize.y };
					GetRenderInterfaceGL(m_RenderInterface)->SetViewport(dimensions.x, dimensions.y);
					m_pContext->SetDimensions(dimensions);
				}
			});

		return 0;
	}

	Ref<RmlUIDocument> UISystem::LoadUIDocument(const String& path)
	{
		Rml::ElementDocument* document;
		if (document = m_pContext->LoadDocument(path))
		{
			// 检查是否已经加载过该文档, 在脚本使用AddUpdateCallback时会事先注册好
			if (auto result = FindDocumentByElementDocument(document))
			{
				result->SetResourcePath(path);
				return result;
			}

			auto uiDocument = CreateRef<RmlUIDocument>();
			uiDocument->document = document;

			uiDocument->SetResourcePath(path);
			m_Documents.push_back(uiDocument);
			return uiDocument;
		}

		return nullptr;
	}

	bool UISystem::ShowUIDocument(RmlUIDocument* doc)
	{
		if (doc != nullptr && doc->document != nullptr)
		{
			doc->document->Show();
			return true;
		}

		return false;
	}

	void UISystem::ReloadUIDocument(Ref<RmlUIDocument>& doc)
	{
		CloseAllDocuments();
		if (doc != nullptr && doc->document != nullptr)
		{
			auto path = doc->GetResourcePath();
			//doc->Close();
			Rml::Factory::ClearStyleSheetCache();
			Rml::Factory::ClearTemplateCache();
			//Rml::ReleaseTextures();

			doc = LoadUIDocument(path);
			ShowUIDocument(doc.get());
		}

		return;
	}

	void UISystem::CloseAllDocuments()
	{
		for (auto& doc: m_Documents)
		{
			if (doc && doc->document)
			{
				doc->Close();
			}
		}

		m_Documents.clear();
	}

	Ref<RmlUIDocument> UISystem::FindDocumentByElementDocument(Rml::ElementDocument* document)
	{
		for (auto& doc : m_Documents)
		{
			if (doc && doc->document == document)
			{
				return doc;
			}
		}

		return nullptr;
	}

	Ref<RmlUIDocument> UISystem::OnScriptOpenDocument(Rml::ElementDocument* document)
	{
		auto uiDocument = CreateRef<RmlUIDocument>();
		uiDocument->document = document;

		m_Documents.push_back(uiDocument);

		return uiDocument;
		//m_NativeDocument.push_back(document);
	}

	void UISystem::OnScriptCloseDocument(const Rml::ElementDocument* document)
	{
		// 移除与 document 匹配的文档，同时调用 Close 以释放资源
		m_Documents.erase(std::remove_if(m_Documents.begin(), m_Documents.end(),
			[document](const Ref<RmlUIDocument>& doc) -> bool
			{
				// 移除空引用
				if (!doc)
					return true;

				// 如果匹配到脚本关闭的原生文档，先关闭再移除
				if (doc->document == document)
				{
					if (doc->document)
					{
						// 保护性调用 Close（原实现中 CloseAllDocuments 会调用 Close）
						doc->Close();
					}
					return true;
				}

				return false;
			}), m_Documents.end());
	}

	Rml::SystemInterface* UISystem::GetSystemInterface() const
	{
		return m_SystemInterface;
	}

	Rml::RenderInterface* UISystem::GetRenderInterface() const
	{
		return m_RenderInterface;
	}

	void UISystem::Render()
	{
		BeginFrame();
		RenderContext();
		EndFrame();
	}


	void UISystem::BeginFrame()
	{
		SDL_GL_MakeCurrent(m_Window->GetSDLWindow(), m_Window->GetContext());

		//GetRenderInterfaceGL(m_RenderInterface)->Clear();
		GetRenderInterfaceGL(m_RenderInterface)->BeginFrame();
	}

	void UISystem::RenderContext()
	{
		m_pContext->Render();
	}

	void UISystem::EndFrame()
	{
		GetRenderInterfaceGL(m_RenderInterface)->EndFrame();
	}

	unsigned int UISystem::GetUIRenderResult()
	{
		if (m_RenderInterface == nullptr)
			return 0;
		return GetRenderInterfaceGL(m_RenderInterface)->GetRenderResult().color_tex_buffer;
	}

	void UISystem::OnUpdate()
	{
		// 更新所有UI文档
		for (auto& doc: m_Documents)
		{
			doc->Update();
		}

		// 更新UI上下文
		m_pContext->Update();
	}

	bool UISystem::InitializeUISystem(Horizon::SDL3::OpenGLWindow* window)
	{

		if (!RmlGL3::Initialize())
		{
			Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to initialize OpenGL renderer");
			return false;
		}

		auto* renderInterface = new RenderInterface_GL3_SDL();
		auto* systemInterface = new SystemInterface_SDL();

		if (!renderInterface)
		{
			m_RenderInterface = nullptr;
			Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to initialize OpenGL3 render interface");
			return false;
		}

		int width = m_Window->WindowWidth();
		int height = m_Window->WindowHeight();

		systemInterface->SetWindow(m_Window->GetSDLWindow());
		renderInterface->SetViewport(width, height);

		m_RenderInterface = renderInterface;
		m_SystemInterface = systemInterface;
	}

	bool UISystem::InitializeRuntimeEnvironment()
	{
		// Initializes the shell which provides common functionality used by the included samples.
		if (!Shell::Initialize())
			return false;

		// Install the custom interfaces constructed by the backend before initializing RmlUi.
		Rml::SetSystemInterface(m_SystemInterface);
		Rml::SetRenderInterface(m_RenderInterface);

		// RmlUi initialisation.
		Rml::Initialise();

		//RmlSol::Initialise();

		int width = m_Window->WindowWidth();
		int height = m_Window->WindowHeight();

		// Create the main RmlUi context.
		m_pContext = Rml::CreateContext("main", Rml::Vector2i(width, height));
		if (!m_pContext)
		{
			Rml::Shutdown();
			Shell::Shutdown();
			return false;
		}

		Rml::Debugger::Initialise(m_pContext);
		Shell::LoadFonts();

		return true;
	}

	int UISystem::ProcessEvent(const SDL_Event& event)
	{
		//m_Window->IsCurrentWindowEvent(event.window.windowID);
		//if (!m_Window->IsCurrentWindowEvent(event.window.windowID))
		//	return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		//H_LOG_INFO("event.window.windowID X:%d", event.window.windowID);
		if (m_Viewport)
		{
			// 不是本窗口事件
			if (m_Viewport->GetWindowID() != event.window.windowID)
				return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

			// 当视口关闭了输入
			if (m_Viewport->IsEnableInput() == false)
				return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;

			// 当场景没有播放
			if (GetSceneManager()->IsPlayMode() == false)
				return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		}

		return m_ProcessContextEventFunction(m_pContext, event); 
	}

	bool UISystem::ProcessContextEvent(Rml::Context* context, const SDL_Event& event)
	{
		auto key_down_callback = &Shell::ProcessKeyDownShortcuts;
		SDL_Event& evt = const_cast<SDL_Event&>(event);

		auto GetKey = [](const SDL_Event& event) { return event.key.key; };
		auto GetDisplayScale = [this]() { return SDL_GetWindowDisplayScale(m_Window->GetSDLWindow()); };

		bool propagate_event = true;
		switch (evt.type)
		{
		case SDL_EVENT_KEY_DOWN:
		{
			propagate_event = false;
			const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(evt));
			const int key_modifier = RmlSDL::GetKeyModifierState();
			const float native_dp_ratio = GetDisplayScale();

			// See if we have any global shortcuts that take priority over the context.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true))
				break;
			// Otherwise, hand the event over to the context by calling the input handler as normal.
			if (!RmlSDL::InputEventHandler(context, m_Window->GetSDLWindow(), evt))
				break;
			// The key was not consumed by the context either, try keyboard shortcuts of lower priority.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false))
				break;
		}
		break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		{
			Rml::Vector2i dimensions = { evt.window.data1, evt.window.data2 };
			GetRenderInterfaceGL(m_RenderInterface)->SetViewport(dimensions.x, dimensions.y);
		}
		break;
		default: break;
		}

		if (propagate_event)
			RmlSDL::InputEventHandler(context, m_Window->GetSDLWindow(), evt);

		//if (propagate_event)
		return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
		//return Horizon::SDL3::WINDOW_LAYER_RESULT_NO_PROPAGATE;
	}

	bool UISystem::ProcessContextEventViewport(Rml::Context* context, const SDL_Event& event)
	{
		auto key_down_callback = &Shell::ProcessKeyDownShortcuts;
		SDL_Event& evt = const_cast<SDL_Event&>(event);

		auto GetKey = [](const SDL_Event& event) { return event.key.key; };
		auto GetDisplayScale = [this]() { return SDL_GetWindowDisplayScale(m_Window->GetSDLWindow()); };

		bool propagate_event = true;
		switch (evt.type)
		{
		case SDL_EVENT_KEY_DOWN:
		{
			propagate_event = false;
			const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(GetKey(evt));
			const int key_modifier = RmlSDL::GetKeyModifierState();
			const float native_dp_ratio = GetDisplayScale();

			// See if we have any global shortcuts that take priority over the context.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true))
				break;
			// Otherwise, hand the event over to the context by calling the input handler as normal.
			if (!RmlSDL::InputEventHandler(context, m_Window->GetSDLWindow(),m_Viewport, evt))
				break;
			// The key was not consumed by the context either, try keyboard shortcuts of lower priority.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false))
				break;
		}
		break;
		//case SDL_EVENT_MOUSE_MOTION:
		//{
		//	const float pixel_density = SDL_GetWindowPixelDensity(m_Window->GetSDLWindow());
		//	float x = evt.motion.x * pixel_density;
		//	float y = evt.motion.y * pixel_density;
		//
		//	H_LOG_INFO("SDL X:%f, Y: %f", x, y);
		//	break;
		//}

		//case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		//{
		//	int2 viewportSize = m_Viewport->GetState().ViewportSize;
		//	Rml::Vector2i dimensions = { viewportSize.x, viewportSize.y };
		//	GetRenderInterfaceGL(m_RenderInterface)->SetViewport(dimensions.x, dimensions.y);
		//}
		//break;
		default: break;
		}

		if (propagate_event)
			RmlSDL::InputEventHandler(context, m_Window->GetSDLWindow(), m_Viewport, evt);

		return Horizon::SDL3::WINDOW_LAYER_RESULT_DEFAULT;
	}
}


