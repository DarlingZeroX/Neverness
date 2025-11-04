#include "UI/UISystem.h"
#include <RmlUi/Debugger.h>
#include <SDL3_image/SDL_image.h>
//#include "Galgame/GalGameEngine.h"
#include "Engine/Manager.h"
#include "Galgame/GameLua.h"
#include "Lua/LuaInterface.h"
#include "UI/Rml/Shell.h"
#include "UI/Lua/Lua.h"
#include "UI/Rml/RmlUi_Platform_SDL.h"
#include "UI/Rml/RmlUi_Renderer_GL3.h"
#include "VGEngine/Source/UI/Lua/LuaPlugin.h"

#include "UI/Sol/Sol.h"
#include "VGEngine/Source/UI/Sol/SolPlugin.h"

namespace VisionGal
{
	/**
		Custom render interface example for the SDL/GL3 backend.

		Overloads the OpenGL3 render interface to load textures through SDL_image's built-in texture loading functionality.
	 */
	class RenderInterface_GL3_SDL : public RenderInterface_GL3 {
	public:
		RenderInterface_GL3_SDL() {}

		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override
		{
			Rml::FileInterface* file_interface = Rml::GetFileInterface();
			Rml::FileHandle file_handle = file_interface->Open(source);
			if (!file_handle)
				return {};

			file_interface->Seek(file_handle, 0, SEEK_END);
			const size_t buffer_size = file_interface->Tell(file_handle);
			file_interface->Seek(file_handle, 0, SEEK_SET);

			using Rml::byte;
			Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
			file_interface->Read(buffer.get(), buffer_size, file_handle);
			file_interface->Close(file_handle);

			const size_t i_ext = source.rfind('.');
			Rml::String extension = (i_ext == Rml::String::npos ? Rml::String() : source.substr(i_ext + 1));

#if SDL_MAJOR_VERSION >= 3
			auto CreateSurface = [&]() { return IMG_LoadTyped_IO(SDL_IOFromMem(buffer.get(), int(buffer_size)), 1, extension.c_str()); };
			auto GetSurfaceFormat = [](SDL_Surface* surface) { return surface->format; };
			auto ConvertSurface = [](SDL_Surface* surface, SDL_PixelFormat format) { return SDL_ConvertSurface(surface, format); };
			auto DestroySurface = [](SDL_Surface* surface) { SDL_DestroySurface(surface); };
#else
			auto CreateSurface = [&]() { return IMG_LoadTyped_RW(SDL_RWFromMem(buffer.get(), int(buffer_size)), 1, extension.c_str()); };
			auto GetSurfaceFormat = [](SDL_Surface* surface) { return surface->format->format; };
			auto ConvertSurface = [](SDL_Surface* surface, Uint32 format) { return SDL_ConvertSurfaceFormat(surface, format, 0); };
			auto DestroySurface = [](SDL_Surface* surface) { SDL_FreeSurface(surface); };
#endif

			SDL_Surface* surface = CreateSurface();
			if (!surface)
				return {};

			texture_dimensions = { surface->w, surface->h };

			if (GetSurfaceFormat(surface) != SDL_PIXELFORMAT_RGBA32)
			{
				// Ensure correct format for premultiplied alpha conversion and GenerateTexture below.
				SDL_Surface* converted_surface = ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
				DestroySurface(surface);
				if (!converted_surface)
					return {};

				surface = converted_surface;
			}

			// Convert colors to premultiplied alpha, which is necessary for correct alpha compositing.
			const size_t pixels_byte_size = surface->w * surface->h * 4;
			byte* pixels = static_cast<byte*>(surface->pixels);
			for (size_t i = 0; i < pixels_byte_size; i += 4)
			{
				const byte alpha = pixels[i + 3];
				for (size_t j = 0; j < 3; ++j)
					pixels[i + j] = byte(int(pixels[i + j]) * int(alpha) / 255);
			}

			Rml::TextureHandle texture_handle = RenderInterface_GL3::GenerateTexture({ pixels, pixels_byte_size }, texture_dimensions);

			DestroySurface(surface);

			return texture_handle;
		}
	};

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

	bool UISystem::LoadDocument(std::string doc)
	{
		// Load and show the tutorial document.
		if (Rml::ElementDocument* document = m_pContext->LoadDocument(doc))
		{
			m_NativeDocument.push_back(document);
			document->Show();
			return true;
		}

		return false;
	}

	Ref<RmlUIDocument> UISystem::LoadUIDocument(const String& path)
	{
		Rml::ElementDocument* document;
		if (document = m_pContext->LoadDocument(path))
		{
			auto uiDocument = CreateRef<RmlUIDocument>();
			uiDocument->document = document;

			uiDocument->SetResourcePath(path);
			m_Documents.push_back(uiDocument);
			return uiDocument;
		}

		return nullptr;
	}

	bool UISystem::ShowUIDocument(const Ref<RmlUIDocument>& doc)
	{
		if (doc != nullptr)
		{
			return ShowUIDocument(doc.get());
		}

		return false;
	}

	bool UISystem::ShowUIDocument(const RmlUIDocument* doc)
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
			ShowUIDocument(doc);
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

		for (auto& doc: m_NativeDocument)
		{
			if (doc)
			{
				doc->Close();
			}
		}

		m_Documents.clear();
		m_NativeDocument.clear();
	}

	void UISystem::OnScriptOpenDocument(Rml::ElementDocument* document)
	{
		m_NativeDocument.push_back(document);
	}

	void UISystem::OnScriptCloseDocument(const Rml::ElementDocument* document)
	{
		for (auto& doc : m_Documents)
		{
			if (doc->document == document)
			{
				doc->isClosed = true;
			}
		}

		for (auto& doc : m_NativeDocument)
		{
			if (doc == document)
			{
				doc = nullptr;
			}
		}
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

		//Rml::Lua::Initialise();
		//sol::state_view view(Rml::Lua::LuaPlugin::GetLuaState());
		//VGLuaInterface::Initialise(view);
		//GalGame::GalGameLuaInterface::Initialise(view);

		RmlSol::Initialise();
		sol::state* solState = RmlSol::SolPlugin::GetLuaState();
		VGLuaInterface::Initialise(*solState);
		GalGame::GalGameLuaInterface::Initialise(*solState);

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


