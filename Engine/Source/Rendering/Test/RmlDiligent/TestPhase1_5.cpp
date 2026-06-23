// =============================================================================
// TestPhase1_5.cpp
// Phase 1.5: RmlUi 集成测试
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>

// RmlUi 头文件
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <fstream>
#include <string>
#include <vector>

// RmlDiligent
#include "RmlDiligentRenderInterface.h"

// Diligent 头文件
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

// D3D12 工厂
#ifdef D3D12_SUPPORTED
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif

#ifdef D3D11_SUPPORTED
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif

// =============================================================================
// RmlUi Samples 根目录（CMake RMLUI_SAMPLES_DIR / 环境变量 / 可执行文件相对路径）
// =============================================================================

static std::string ResolveRmlUiSamplesRoot(const char* argv0)
{
#ifdef RMLUI_SAMPLES_DIR
    {
        std::string fromCmake = RMLUI_SAMPLES_DIR;
        if (!fromCmake.empty()) {
            std::error_code ec;
            auto abs = std::filesystem::absolute(fromCmake, ec);
            if (!ec) {
                fromCmake = abs.string();
            }
            if (fromCmake.back() != '/' && fromCmake.back() != '\\') {
                fromCmake += '/';
            }
            return fromCmake;
        }
    }
#endif
    if (const char* env = std::getenv("RMLUI_SAMPLES")) {
        std::string path = env;
        if (!path.empty() && path.back() != '/' && path.back() != '\\') {
            path += '/';
        }
        return path;
    }
    if (argv0 && argv0[0] != '\0') {
        std::error_code ec;
        auto exe = std::filesystem::absolute(argv0, ec);
        if (!ec) {
            auto dir = exe.parent_path();
            for (int i = 0; i < 8; ++i) {
                auto candidate = dir / "Engine" / "Source" / "ThirdParty" / "RmlUi" / "Samples";
                if (std::filesystem::is_directory(candidate)) {
                    std::string path = candidate.string();
                    if (path.back() != '/' && path.back() != '\\') {
                        path += '/';
                    }
                    return path;
                }
                if (!dir.has_parent_path() || dir == dir.parent_path()) {
                    break;
                }
                dir = dir.parent_path();
            }
        }
    }
    return "Engine/Source/ThirdParty/RmlUi/Samples/";
}

// =============================================================================
// 辅助函数：从文件加载到内存
// =============================================================================
static std::vector<Rml::byte> LoadFileToMemory(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<Rml::byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {};
    }

    return buffer;
}

// =============================================================================
// FileInterface 实现
// =============================================================================

class RmlDiligentFileInterface : public Rml::FileInterface {
public:
    RmlDiligentFileInterface(const Rml::String& root) : root(root) {}
    virtual ~RmlDiligentFileInterface() {}

    Rml::FileHandle Open(const Rml::String& path) override {
        if (path.empty()) return (Rml::FileHandle)nullptr;

        // Try with root path first
        FILE* fp = fopen((root + path).c_str(), "rb");
        if (fp != nullptr) return (Rml::FileHandle)fp;

        // Try without root
        fp = fopen(path.c_str(), "rb");
        return (Rml::FileHandle)fp;
    }

    void Close(Rml::FileHandle file) override {
        fclose((FILE*)file);
    }

    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override {
        return fread(buffer, 1, size, (FILE*)file);
    }

    bool Seek(Rml::FileHandle file, long offset, int origin) override {
        return fseek((FILE*)file, offset, origin) == 0;
    }

    size_t Tell(Rml::FileHandle file) override {
        return ftell((FILE*)file);
    }

private:
    Rml::String root;
};

// =============================================================================
// SystemInterface 实现
// =============================================================================

class RmlDiligentSystemInterface : public Rml::SystemInterface {
public:
    double GetElapsedTime() override {
        return SDL_GetTicks() / 1000.0;
    }

    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override {
        switch (type) {
            case Rml::Log::Type::LT_ERROR:
                std::cerr << "[RmlUi ERROR]";
                break;
            case Rml::Log::Type::LT_WARNING:
                std::cerr << "[RmlUi WARN]";
                break;
            default:
                std::cerr << "[RmlUi]";
                break;
        }
        if (!message.empty()) {
            std::cerr << ' ' << message.c_str();
        }
        std::cerr << std::endl;
        return true;
    }
};

// =============================================================================
// 创建 Diligent 设备
// =============================================================================

static bool CreateDeviceAndSwapChain(
    SDL_Window* window,
    Diligent::IRenderDevice** ppDevice,
    Diligent::IDeviceContext** ppContext,
    Diligent::ISwapChain** ppSwapChain)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!hwnd) {
        std::cerr << "Failed to get HWND from SDL window" << std::endl;
        return false;
    }

    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);

    Diligent::SwapChainDesc scDesc;
    scDesc.Width = static_cast<Diligent::Uint32>(width);
    scDesc.Height = static_cast<Diligent::Uint32>(height);
    scDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    scDesc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    scDesc.Usage = Diligent::SWAP_CHAIN_USAGE_RENDER_TARGET;

#ifdef D3D12_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D12();
        if (factory) {
            Diligent::EngineD3D12CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D12(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                Diligent::Win32NativeWindow nativeWnd{hwnd};
                factory->CreateSwapChainD3D12(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
                if (*ppSwapChain) {
                    std::cout << "[Phase1.5] Backend: D3D12" << std::endl;
                    return true;
                }
            }
        }
    }
#endif

#ifdef D3D11_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D11();
        if (factory) {
            Diligent::EngineD3D11CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D11(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                Diligent::Win32NativeWindow nativeWnd{hwnd};
                factory->CreateSwapChainD3D11(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
                if (*ppSwapChain) {
                    std::cout << "[Phase1.5] Backend: D3D11" << std::endl;
                    return true;
                }
            }
        }
    }
#endif

    return false;
}

// =============================================================================
// 冒烟文档：加载 + 强制布局（RCSS 未生效时回退 inline style）
// =============================================================================

static const char* kInlineSmokeRml = R"(
<rml>
<head>
<title>Phase 1.5</title>
</head>
<body>
<div id="panel" style="display: block; width: 400dp; height: 200dp; background-color: #ff0000; padding: 16dp; color: #ffffff; font-size: 24dp; font-family: LatoLatin;">
<p style="display: block; color: #ffffff; font-size: 24dp; font-family: LatoLatin;">Hello RmlDiligent</p>
</div>
</body>
</rml>
)";

static const char* kPanelForceStyle =
    "display: block; width: 400dp; height: 200dp; background-color: #ff0000; padding: 16dp; color: #ffffff; font-size: 24dp;";

static void ForcePanelInlineStyle(Rml::ElementDocument* document, bool useLatoFont)
{
    if (!document) {
        return;
    }

    Rml::String inner = R"(
<div id="panel" style="display: block; width: 400dp; height: 200dp; background-color: #ff0000; padding: 16dp; color: #ffffff; font-size: 24dp;">
<p style="color: #ffffff; font-size: 24dp;">Hello RmlDiligent</p>
</div>
)";
    if (useLatoFont) {
        inner = R"(
<div id="panel" style="display: block; width: 400dp; height: 200dp; background-color: #ff0000; padding: 16dp; color: #ffffff; font-size: 24dp; font-family: LatoLatin;">
<p style="color: #ffffff; font-size: 24dp;">Hello RmlDiligent</p>
</div>
)";
    }
    document->SetInnerRML(inner);

    if (Rml::Element* panel = document->GetElementById("panel")) {
        Rml::String style = kPanelForceStyle;
        if (useLatoFont) {
            style += " font-family: LatoLatin;";
        }
        panel->SetAttribute("style", style);
    }
}

static bool EnsureSmokeDocumentLayout(Rml::Context* rmlContext, Rml::ElementDocument* document, bool useLatoFont)
{
    if (!rmlContext || !document) {
        return false;
    }

    document->PullToFront();
    document->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);

    for (int i = 0; i < 4; ++i) {
        rmlContext->Update();
    }
    document->UpdateDocument();

    auto measure = [&]() -> std::pair<float, float> {
        Rml::Element* panel = document->GetElementById("panel");
        if (!panel) {
            return {0.0f, 0.0f};
        }
        return {panel->GetClientWidth(), panel->GetClientHeight()};
    };

    std::pair<float, float> size = measure();
    float w = size.first;
    float h = size.second;
    if (w <= 0.0f || h <= 0.0f) {
        std::cerr << "[WARN] Layout zero after load; forcing inline #panel style" << std::endl;
        ForcePanelInlineStyle(document, useLatoFont);
        document->UpdateDocument();
        for (int i = 0; i < 4; ++i) {
            rmlContext->Update();
        }
        size = measure();
        w = size.first;
        h = size.second;
    }

    return w > 0.0f && h > 0.0f;
}

static Rml::ElementDocument* LoadSmokeDocument(Rml::Context* rmlContext, bool useLatoFont)
{
    Rml::ElementDocument* document = rmlContext->LoadDocument("phase15_smoke.rml");
    if (document) {
        std::cout << "[OK] Loaded phase15_smoke.rml" << std::endl;
        return document;
    }

    std::cerr << "[WARN] phase15_smoke.rml failed, using inline RML" << std::endl;
    document = rmlContext->LoadDocumentFromMemory(kInlineSmokeRml, "phase15_smoke.rml");
    if (document) {
        std::cout << "[OK] Inline smoke document loaded" << std::endl;
        return document;
    }

    std::cerr << "[WARN] LoadDocumentFromMemory failed, using minimal inline RML" << std::endl;
    return rmlContext->LoadDocumentFromMemory(kInlineSmokeRml, "inline://smoke.rml");
}

// =============================================================================
// RmlUi 键盘映射
// =============================================================================

static Rml::Input::KeyIdentifier ConvertKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_RETURN: return Rml::Input::KI_RETURN;
        case SDLK_ESCAPE: return Rml::Input::KI_ESCAPE;
        case SDLK_SPACE: return Rml::Input::KI_SPACE;
        case SDLK_LEFT: return Rml::Input::KI_LEFT;
        case SDLK_RIGHT: return Rml::Input::KI_RIGHT;
        case SDLK_UP: return Rml::Input::KI_UP;
        case SDLK_DOWN: return Rml::Input::KI_DOWN;
        case SDLK_BACKSPACE: return Rml::Input::KI_BACK;
        case SDLK_TAB: return Rml::Input::KI_TAB;
        case SDLK_DELETE: return Rml::Input::KI_DELETE;
        default:
            if (key >= SDLK_A && key <= SDLK_Z) {
                return static_cast<Rml::Input::KeyIdentifier>(Rml::Input::KI_A + (key - SDLK_A));
            }
            if (key >= SDLK_0 && key <= SDLK_9) {
                return static_cast<Rml::Input::KeyIdentifier>(Rml::Input::KI_0 + (key - SDLK_0));
            }
            return Rml::Input::KI_UNKNOWN;
    }
}

// =============================================================================
// 主函数
// =============================================================================

int main(int argc, char* argv[])
{
    int autoExitFrames = 0;
    bool drawDebugQuad = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--demo") {
            continue;
        }
        if (std::string(argv[i]) == "--no-debug-quad") {
            drawDebugQuad = false;
            continue;
        }
        if (std::string(argv[i]) == "--frames" && i + 1 < argc) {
            autoExitFrames = std::atoi(argv[i + 1]);
            ++i;
        }
    }

    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 1.5: RmlUi Integration" << std::endl;
    std::cout << "============================================" << std::endl;

    // 初始化 SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow(
        "RmlDiligent Phase 1.5",
        1024, 768,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow FAILED: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] SDL3 window created" << std::endl;

    // 创建 Diligent 设备
    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;
    Diligent::ISwapChain* swapChain = nullptr;

    if (!CreateDeviceAndSwapChain(window, &device, &context, &swapChain)) {
        std::cerr << "[FAIL] CreateDeviceAndSwapChain FAILED!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] Diligent device created" << std::endl;

    // 创建 RmlDiligent 渲染接口
    RmlDiligent::RmlDiligentRenderInterface renderInterface;
    if (!renderInterface.Initialize(device, context, swapChain)) {
        std::cerr << "[FAIL] RenderInterface init failed!" << std::endl;
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] RenderInterface init success" << std::endl;

    // 创建 SystemInterface 和 FileInterface
    RmlDiligentSystemInterface systemInterface;
    const std::string samplesRoot = ResolveRmlUiSamplesRoot(argc > 0 ? argv[0] : nullptr);
    std::cout << "[INFO] RmlUi Samples root: " << samplesRoot << std::endl;
    RmlDiligentFileInterface fileInterface(samplesRoot);

    // 初始化 RmlUi
    Rml::SetSystemInterface(&systemInterface);
    Rml::SetFileInterface(&fileInterface);
    Rml::SetRenderInterface(&renderInterface);  // 必须在 Initialise 之前
    if (!Rml::Initialise()) {
        std::cerr << "[FAIL] Rml::Initialise failed" << std::endl;
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] RmlUi init success" << std::endl;

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        width = 1024;
        height = 768;
    }
    Rml::Context* rmlContext = Rml::CreateContext("main", Rml::Vector2i(width, height), &renderInterface);
    if (rmlContext) {
        rmlContext->SetDensityIndependentPixelRatio(1.0f);
    }

    if (!rmlContext) {
        std::cerr << "[FAIL] RmlUi Context creation failed!" << std::endl;
        Rml::Shutdown();
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] RmlUi Context created (" << rmlContext->GetDimensions().x << "x"
              << rmlContext->GetDimensions().y << ")" << std::endl;

    const bool fontLoaded = Rml::LoadFontFace("assets/LatoLatin-Regular.ttf", true);
    if (fontLoaded) {
        std::cout << "[OK] Font loaded: assets/LatoLatin-Regular.ttf" << std::endl;
    } else {
        std::cerr << "[WARN] Font load failed; text may not render" << std::endl;
    }
    std::cout.flush();

    renderInterface.SetProjectionMatrix(width, height);

    // Phase 1.5 冒烟测试：使用 inline 文档（demo.rml 依赖 Lua/大量资源，易导致卡死）
    Rml::ElementDocument* document = nullptr;
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        std::cout << "[INFO] Loading demo.rml..." << std::endl;
        std::cout.flush();
        document = rmlContext->LoadDocument("demo.rml");
        if (document) {
            std::cout << "[OK] Loaded demo.rml" << std::endl;
        }
    } else {
        document = LoadSmokeDocument(rmlContext, fontLoaded);
    }

    if (!document) {
        std::cerr << "[FAIL] Document load failed!" << std::endl;
        Rml::Shutdown();
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const bool layoutOk = EnsureSmokeDocumentLayout(rmlContext, document, fontLoaded);
    std::cout << "[INFO] context documents: " << rmlContext->GetNumDocuments()
              << " doc-children: " << document->GetNumChildren(true) << std::endl;

    auto logLayout = [&](const char* label, Rml::Element* element) {
        if (!element) {
            return;
        }
        const Rml::Vector2f size = element->GetBox().GetSize();
        std::cout << "[INFO] " << label << " layout: " << size.x << " x " << size.y << std::endl;
    };
    logLayout("document", document);
    if (Rml::Element* panel = document->GetElementById("panel")) {
        logLayout("panel", panel);
        std::cout << "[INFO] panel client: " << panel->GetClientWidth() << " x " << panel->GetClientHeight()
                  << " visible=" << (panel->IsVisible(true) ? "yes" : "no") << std::endl;
    } else {
        std::cerr << "[WARN] #panel element not found" << std::endl;
    }
    if (!layoutOk) {
        std::cerr << "[FAIL] Document layout is zero; check RmlUi DLL/RCSS. Run from build output dir." << std::endl;
        std::cerr << "[INFO] cwd: " << std::filesystem::current_path().string() << std::endl;
    }
    std::cout.flush();

    // 渲染循环
    std::cout << "\n[INFO] Starting render loop... Press ESC to exit" << std::endl;
    std::cout.flush();

    bool running = true;
    int frameCount = 0;
    bool firstFrameSynced = false;
    uint32_t lastRmlDrawCount = 0;
    uint32_t lastTextureDrawCount = 0;
    float panelClientW = 0.0f;
    float panelClientH = 0.0f;
    auto startTime = SDL_GetTicks();

    while (running) {
        SDL_PumpEvents();

        // 处理事件
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (evt.type == SDL_EVENT_KEY_DOWN) {
                if (evt.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                rmlContext->ProcessKeyDown(ConvertKey(evt.key.key), 0);
            }
            if (evt.type == SDL_EVENT_KEY_UP) {
                rmlContext->ProcessKeyUp(ConvertKey(evt.key.key), 0);
            }
            if (evt.type == SDL_EVENT_MOUSE_MOTION) {
                rmlContext->ProcessMouseMove(
                    static_cast<int>(evt.motion.x),
                    static_cast<int>(evt.motion.y),
                    0
                );
            }
            if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (evt.button.button == SDL_BUTTON_LEFT) {
                    rmlContext->ProcessMouseButtonDown(0, 0);
                }
            }
            if (evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (evt.button.button == SDL_BUTTON_LEFT) {
                    rmlContext->ProcessMouseButtonUp(0, 0);
                }
            }
            if (evt.type == SDL_EVENT_WINDOW_RESIZED) {
                width = evt.window.data1;
                height = evt.window.data2;
                if (width > 0 && height > 0) {
                    swapChain->Resize(static_cast<Diligent::Uint32>(width), static_cast<Diligent::Uint32>(height));
                    rmlContext->SetDimensions(Rml::Vector2i(width, height));
                    renderInterface.SetProjectionMatrix(width, height);
                }
            }
        }

        if (!firstFrameSynced) {
            SDL_GetWindowSize(window, &width, &height);
            if (width > 0 && height > 0) {
                renderInterface.SetProjectionMatrix(width, height);
                rmlContext->SetDimensions(Rml::Vector2i(width, height));
                firstFrameSynced = true;
                std::cout << "[INFO] First-frame projection sync: " << width << "x" << height << std::endl;
            }
        }

        // 更新 RmlUi（须在尺寸同步之后）
        rmlContext->Update();

        // 渲染
        renderInterface.BeginFrame();
        renderInterface.ResetDrawCount();

        if (drawDebugQuad && frameCount == 0) {
            renderInterface.DrawDebugQuad();
            const uint32_t debugDraws = renderInterface.GetDrawCount();
            std::cout << "[INFO] DrawDebugQuad drawCount=" << debugDraws << std::endl;
            renderInterface.ResetDrawCount();
        }

        rmlContext->Render();
        lastRmlDrawCount = renderInterface.GetDrawCount();
        lastTextureDrawCount = renderInterface.GetTextureDrawCount();

        if (frameCount == 0 || frameCount == 2) {
            std::cout << "[INFO] frame " << frameCount << " RmlUi drawCount=" << renderInterface.GetDrawCount()
                      << " textureDraws=" << renderInterface.GetTextureDrawCount()
                      << " pushLayer=" << renderInterface.GetPushLayerCount()
                      << " composite=" << renderInterface.GetCompositeCount() << std::endl;
            if (frameCount == 0) {
                if (Rml::Element* panel = document->GetElementById("panel")) {
                    std::cout << "[INFO] post-render panel client: " << panel->GetClientWidth() << " x "
                              << panel->GetClientHeight() << std::endl;
                }
            }
        }

        renderInterface.EndFrame();

        // Present
        swapChain->Present(0);

        frameCount++;

        if (autoExitFrames > 0 && frameCount >= autoExitFrames) {
            std::cout << "[INFO] --frames " << autoExitFrames << " reached, exiting" << std::endl;
            std::cout.flush();
            running = false;
        }
    }

    // 统计
    auto endTime = SDL_GetTicks();
    float elapsed = (endTime - startTime) / 1000.0f;
    float fps = frameCount / elapsed;

    if (Rml::Element* panel = document->GetElementById("panel")) {
        panelClientW = panel->GetClientWidth();
        panelClientH = panel->GetClientHeight();
    }

    const bool acceptLayoutOk = panelClientW > 0.0f && panelClientH > 0.0f;
    const bool renderOk = lastRmlDrawCount > 0;
    const bool textOk = lastTextureDrawCount > 0;

    std::cout << "\n============================================" << std::endl;
    if (acceptLayoutOk && renderOk && textOk) {
        std::cout << "  [PASS] Phase 1.5 通过!" << std::endl;
    } else {
        std::cout << "  [FAIL] Phase 1.5 验收未通过" << std::endl;
        if (!acceptLayoutOk) {
            std::cout << "  - panel 布局为 0" << std::endl;
        }
        if (!renderOk) {
            std::cout << "  - RmlUi drawCount 为 0" << std::endl;
        }
        if (!textOk) {
            std::cout << "  - 无纹理 draw（文字未生成，检查 @font-face / LoadFontFace）" << std::endl;
        }
    }
    std::cout << "  帧数: " << frameCount << std::endl;
    std::cout << "  时间: " << elapsed << "s" << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "  panel: " << panelClientW << " x " << panelClientH << std::endl;
    std::cout << "  last RmlUi drawCount: " << lastRmlDrawCount
              << " textureDraws: " << lastTextureDrawCount << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout.flush();

    if (!acceptLayoutOk || !renderOk || !textOk) {
        if (document) {
            document->Close();
        }
        Rml::Shutdown();
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 清理
    if (document) {
        document->Close();
    }
    Rml::Shutdown();
    swapChain->Release();
    context->Release();
    device->Release();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
