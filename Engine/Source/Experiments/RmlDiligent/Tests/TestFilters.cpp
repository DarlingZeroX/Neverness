// =============================================================================
// TestFilters.cpp
// Phase 5b: Filters 验证
// =============================================================================

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>

#include "RmlDiligentRenderInterface.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#ifdef D3D12_SUPPORTED
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif

#ifdef D3D11_SUPPORTED
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif

enum class FiltersTestCase {
    All,
    Opacity,
    Blur,
    Shadow,
    Brightness,
    Contrast,
    Stack,
};

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

class RmlDiligentFileInterface : public Rml::FileInterface {
public:
    explicit RmlDiligentFileInterface(const Rml::String& root) : root(root) {}

    Rml::FileHandle Open(const Rml::String& path) override {
        if (path.empty()) {
            return (Rml::FileHandle)nullptr;
        }
        FILE* fp = fopen((root + path).c_str(), "rb");
        if (fp != nullptr) {
            return (Rml::FileHandle)fp;
        }
        return (Rml::FileHandle)fopen(path.c_str(), "rb");
    }

    void Close(Rml::FileHandle file) override { fclose((FILE*)file); }
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override {
        return fread(buffer, 1, size, (FILE*)file);
    }
    bool Seek(Rml::FileHandle file, long offset, int origin) override {
        return fseek((FILE*)file, offset, origin) == 0;
    }
    size_t Tell(Rml::FileHandle file) override { return ftell((FILE*)file); }

private:
    Rml::String root;
};

class RmlDiligentSystemInterface : public Rml::SystemInterface {
public:
    double GetElapsedTime() override { return SDL_GetTicks() / 1000.0; }

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

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);

    Diligent::SwapChainDesc scDesc;
    scDesc.Width = static_cast<Diligent::Uint32>(width);
    scDesc.Height = static_cast<Diligent::Uint32>(height);
    scDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    scDesc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    scDesc.Usage = Diligent::SWAP_CHAIN_USAGE_RENDER_TARGET;

#ifdef D3D12_SUPPORTED
    if (auto* factory = Diligent::GetEngineFactoryD3D12()) {
        Diligent::EngineD3D12CreateInfo engineCI;
        engineCI.EnableValidation = true;
        factory->CreateDeviceAndContextsD3D12(engineCI, ppDevice, ppContext);
        if (*ppDevice) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D12(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
            if (*ppSwapChain) {
                std::cout << "[Phase5b] Backend: D3D12" << std::endl;
                return true;
            }
        }
    }
#endif

#ifdef D3D11_SUPPORTED
    if (auto* factory = Diligent::GetEngineFactoryD3D11()) {
        Diligent::EngineD3D11CreateInfo engineCI;
        engineCI.EnableValidation = true;
        factory->CreateDeviceAndContextsD3D11(engineCI, ppDevice, ppContext);
        if (*ppDevice) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D11(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
            if (*ppSwapChain) {
                std::cout << "[Phase5b] Backend: D3D11" << std::endl;
                return true;
            }
        }
    }
#endif

    return false;
}

static const char* kInlineAllRml = R"(
<rml><head><title>Phase 5b Inline</title></head><body style="display: block; margin: 0; width: 1024dp; height: 768dp; background: #1a1a2e;">
<div id="opacity-panel" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 200dp; height: 120dp; padding: 10dp; background: #3366cc; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: opacity(0.55);">opacity</div>
<div id="blur-panel" style="display: block; position: absolute; left: 280dp; top: 40dp; width: 200dp; height: 120dp; padding: 10dp; background: #22aa66; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: blur(4dp);">blur</div>
<div id="shadow-panel" style="display: block; position: absolute; left: 520dp; top: 40dp; width: 200dp; height: 120dp; padding: 10dp; background: #cc6633; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: drop-shadow(#000000aa 2dp 4dp 6dp);">drop-shadow</div>
<div id="stack-panel" style="display: block; position: absolute; left: 760dp; top: 40dp; width: 200dp; height: 120dp; padding: 10dp; background: #2288aa; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: brightness(1.25) contrast(1.5);">stack</div>
<div id="bright-panel" style="display: block; position: absolute; left: 40dp; top: 200dp; width: 200dp; height: 120dp; padding: 10dp; background: #8844aa; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: brightness(1.4);">brightness</div>
<div id="contrast-panel" style="display: block; position: absolute; left: 520dp; top: 200dp; width: 200dp; height: 120dp; padding: 10dp; background: #4488cc; color: #ffffff; font-size: 14dp; font-family: LatoLatin; filter: contrast(1.6);">contrast</div>
<p id="ref" style="display: block; position: absolute; left: 280dp; top: 200dp; width: 200dp; height: 60dp; background: #444444; color: #cccccc; font-size: 14dp; font-family: LatoLatin;">reference</p>
</body></rml>
)";

static const char* InlineRmlForCase(FiltersTestCase testCase)
{
    (void)testCase;
    return kInlineAllRml;
}

static std::vector<const char*> RequiredElementIds(FiltersTestCase testCase)
{
    switch (testCase) {
        case FiltersTestCase::Opacity:
            return {"opacity-panel", "ref"};
        case FiltersTestCase::Blur:
            return {"blur-panel", "ref"};
        case FiltersTestCase::Shadow:
            return {"shadow-panel", "ref"};
        case FiltersTestCase::Brightness:
            return {"bright-panel", "ref"};
        case FiltersTestCase::Contrast:
            return {"contrast-panel", "ref"};
        case FiltersTestCase::Stack:
            return {"stack-panel", "ref"};
        case FiltersTestCase::All:
        default:
            return {"opacity-panel", "blur-panel", "shadow-panel", "stack-panel",
                    "bright-panel", "contrast-panel", "ref"};
    }
}

static uint32_t MinFilterRenderCount(FiltersTestCase testCase)
{
    switch (testCase) {
        case FiltersTestCase::Stack:
            return 2;
        case FiltersTestCase::All:
            return 6;
        case FiltersTestCase::Shadow:
            return 1;
        case FiltersTestCase::Blur:
            return 1;
        default:
            return 1;
    }
}

static Rml::ElementDocument* LoadFiltersDocument(Rml::Context* rmlContext, FiltersTestCase testCase)
{
    (void)testCase;
    if (Rml::ElementDocument* document = rmlContext->LoadDocument("phase5_filters.rml")) {
        std::cout << "[OK] Loaded phase5_filters.rml" << std::endl;
        return document;
    }
    std::cerr << "[WARN] phase5_filters.rml failed, using inline RML" << std::endl;
    return rmlContext->LoadDocumentFromMemory(InlineRmlForCase(testCase), "phase5_filters.rml");
}

static bool EnsureDocumentLayout(Rml::Context* rmlContext, Rml::ElementDocument* document)
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
    return true;
}

static Rml::Input::KeyIdentifier ConvertKey(SDL_Keycode key)
{
    switch (key) {
        case SDLK_ESCAPE:
            return Rml::Input::KI_ESCAPE;
        default:
            return Rml::Input::KI_UNKNOWN;
    }
}

static FiltersTestCase ParseTestCase(const std::string& value)
{
    if (value == "opacity") {
        return FiltersTestCase::Opacity;
    }
    if (value == "blur") {
        return FiltersTestCase::Blur;
    }
    if (value == "shadow") {
        return FiltersTestCase::Shadow;
    }
    if (value == "brightness") {
        return FiltersTestCase::Brightness;
    }
    if (value == "contrast") {
        return FiltersTestCase::Contrast;
    }
    if (value == "stack") {
        return FiltersTestCase::Stack;
    }
    return FiltersTestCase::All;
}

static const char* TestCaseName(FiltersTestCase testCase)
{
    switch (testCase) {
        case FiltersTestCase::Opacity:
            return "opacity";
        case FiltersTestCase::Blur:
            return "blur";
        case FiltersTestCase::Shadow:
            return "shadow";
        case FiltersTestCase::Brightness:
            return "brightness";
        case FiltersTestCase::Contrast:
            return "contrast";
        case FiltersTestCase::Stack:
            return "stack";
        case FiltersTestCase::All:
        default:
            return "all";
    }
}

int main(int argc, char* argv[])
{
    int autoExitFrames = 0;
    FiltersTestCase testCase = FiltersTestCase::All;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) {
            autoExitFrames = std::atoi(argv[++i]);
        } else if (arg == "--case" && i + 1 < argc) {
            testCase = ParseTestCase(argv[++i]);
        }
    }

    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 5b: Filters" << std::endl;
    std::cout << "  case: " << TestCaseName(testCase) << std::endl;
    std::cout << "============================================" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("RmlDiligent Phase 5b", 1024, 768, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow FAILED: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] SDL3 window created" << std::endl;

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

    RmlDiligentSystemInterface systemInterface;
    const std::string samplesRoot = ResolveRmlUiSamplesRoot(argc > 0 ? argv[0] : nullptr);
    std::cout << "[INFO] RmlUi Samples root: " << samplesRoot << std::endl;
    RmlDiligentFileInterface fileInterface(samplesRoot);

    Rml::SetSystemInterface(&systemInterface);
    Rml::SetFileInterface(&fileInterface);
    Rml::SetRenderInterface(&renderInterface);
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
    rmlContext->SetDensityIndependentPixelRatio(1.0f);
    std::cout << "[OK] RmlUi Context created (" << width << "x" << height << ")" << std::endl;

    const bool fontLoaded = Rml::LoadFontFace("assets/LatoLatin-Regular.ttf", true);
    if (fontLoaded) {
        std::cout << "[OK] Font loaded: assets/LatoLatin-Regular.ttf" << std::endl;
    } else {
        std::cerr << "[WARN] Font load failed; text may not render" << std::endl;
    }

    renderInterface.SetProjectionMatrix(width, height);

    Rml::ElementDocument* document = LoadFiltersDocument(rmlContext, testCase);
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

    EnsureDocumentLayout(rmlContext, document);

    const auto requiredIds = RequiredElementIds(testCase);
    size_t elementsFound = 0;
    size_t elementsWithLayout = 0;

    for (const char* id : requiredIds) {
        Rml::Element* element = document->GetElementById(id);
        if (!element) {
            std::cerr << "[WARN] #" << id << " not found" << std::endl;
            continue;
        }
        ++elementsFound;
        const Rml::Vector2f size = element->GetBox().GetSize();
        const float clientW = element->GetClientWidth();
        const float clientH = element->GetClientHeight();
        std::cout << "[INFO] #" << id << " layout: " << size.x << " x " << size.y
                  << " client: " << clientW << " x " << clientH
                  << " visible=" << (element->IsVisible(true) ? "yes" : "no") << std::endl;
        if (size.x > 0.0f && size.y > 0.0f) {
            ++elementsWithLayout;
        }
    }

    const size_t minDrawCount = requiredIds.size();
    std::cout << "\n[INFO] Visual checklist:" << std::endl;
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Opacity) {
        std::cout << "  - #opacity-panel: filter: opacity(0.55)" << std::endl;
    }
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Blur) {
        std::cout << "  - #blur-panel: filter: blur(4dp)" << std::endl;
    }
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Shadow) {
        std::cout << "  - #shadow-panel: filter: drop-shadow" << std::endl;
    }
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Brightness) {
        std::cout << "  - #bright-panel: filter: brightness(1.4)" << std::endl;
    }
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Contrast) {
        std::cout << "  - #contrast-panel: filter: contrast(1.6)" << std::endl;
    }
    if (testCase == FiltersTestCase::All || testCase == FiltersTestCase::Stack) {
        std::cout << "  - #stack-panel: filter: brightness(1.25) contrast(1.5)" << std::endl;
    }
    std::cout << "\n[INFO] Starting render loop... Press ESC to exit (use --frames N for auto-exit)" << std::endl;

    bool running = true;
    int frameCount = 0;
    bool firstFrameSynced = false;
    uint32_t lastRmlDrawCount = 0;
    uint32_t lastPushLayerCount = 0;
    uint32_t lastCompositeCount = 0;
    uint32_t lastFilterRenderCount = 0;
    auto startTime = SDL_GetTicks();

    while (running) {
        SDL_PumpEvents();
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT || (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_ESCAPE)) {
                running = false;
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

        rmlContext->Update();
        renderInterface.BeginFrame();
        renderInterface.ResetDrawCount();
        rmlContext->Render();
        lastRmlDrawCount = renderInterface.GetDrawCount();
        lastPushLayerCount = renderInterface.GetPushLayerCount();
        lastCompositeCount = renderInterface.GetCompositeCount();
        lastFilterRenderCount = renderInterface.GetFilterRenderCount();

        if (frameCount == 0 || frameCount == 2) {
            std::cout << "[INFO] frame " << frameCount << " drawCount=" << lastRmlDrawCount
                      << " pushLayers=" << lastPushLayerCount << " composites=" << lastCompositeCount
                      << " filterRenders=" << lastFilterRenderCount << std::endl;
        }

        renderInterface.EndFrame();
        swapChain->Present(0);
        ++frameCount;

        if (autoExitFrames > 0 && frameCount >= autoExitFrames) {
            std::cout << "[INFO] --frames " << autoExitFrames << " reached, exiting" << std::endl;
            running = false;
        }
    }

    const float elapsed = (SDL_GetTicks() - startTime) / 1000.0f;
    const float fps = frameCount > 0 ? frameCount / elapsed : 0.0f;

    const bool elementsOk = elementsFound == requiredIds.size() && elementsWithLayout == requiredIds.size();
    const bool renderOk = lastRmlDrawCount >= minDrawCount;
    const bool layerOk = lastPushLayerCount > 0 && lastCompositeCount > 0;
    const uint32_t minFilterRenders = MinFilterRenderCount(testCase);
    const bool filterOk = lastFilterRenderCount >= minFilterRenders;

    std::cout << "\n============================================" << std::endl;
    if (elementsOk && renderOk && layerOk && filterOk) {
        std::cout << "  [PASS] Phase 5b 通过!" << std::endl;
    } else {
        std::cout << "  [FAIL] Phase 5b 验收未通过" << std::endl;
        if (!elementsOk) {
            std::cout << "  - 测试元素缺失或 layout 为 0 (" << elementsFound << "/" << requiredIds.size()
                      << " found, " << elementsWithLayout << " with layout)" << std::endl;
        }
        if (!renderOk) {
            std::cout << "  - drawCount=" << lastRmlDrawCount << " (need >= " << minDrawCount << ")" << std::endl;
        }
        if (!layerOk) {
            std::cout << "  - pushLayers=" << lastPushLayerCount << " composites=" << lastCompositeCount
                      << " (need both > 0)" << std::endl;
        }
        if (!filterOk) {
            std::cout << "  - filterRenders=" << lastFilterRenderCount << " (need >= " << minFilterRenders << ")" << std::endl;
        }
    }
    std::cout << "  case: " << TestCaseName(testCase) << std::endl;
    std::cout << "  帧数: " << frameCount << std::endl;
    std::cout << "  时间: " << elapsed << "s" << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "  last drawCount: " << lastRmlDrawCount << " pushLayers: " << lastPushLayerCount
              << " composites: " << lastCompositeCount << " filterRenders: " << lastFilterRenderCount << std::endl;
    std::cout << "============================================" << std::endl;

    document->Close();
    Rml::Shutdown();
    swapChain->Release();
    context->Release();
    device->Release();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return (elementsOk && renderOk && layerOk && filterOk) ? 0 : 1;
}
