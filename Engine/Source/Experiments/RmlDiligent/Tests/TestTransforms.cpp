// =============================================================================
// TestTransforms.cpp
// Phase 2: CSS Transform 属性验证
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

enum class TransformTestCase {
    All,
    Rotate,
    Scale,
    Translate,
    Nested,
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
                std::cout << "[Phase2] Backend: D3D12" << std::endl;
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
                std::cout << "[Phase2] Backend: D3D11" << std::endl;
                return true;
            }
        }
    }
#endif

    return false;
}

static const char* kInlineAllRml = R"(
<rml>
<head><title>Phase 2 Inline</title></head>
<body>
<div id="ref" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #0066cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin;"><p>Ref</p></div>
<div id="rotate" style="display: block; position: absolute; left: 240dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #00aa44; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: rotate(45deg);"><p>Rotate</p></div>
<div id="scale" style="display: block; position: absolute; left: 440dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #ccaa00; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: scale(1.5);"><p>Scale</p></div>
<div id="translate" style="display: block; position: absolute; left: 640dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #8800cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: translate(80dp, 40dp);"><p>Move</p></div>
<div id="parent" style="display: block; position: absolute; left: 40dp; top: 280dp; width: 160dp; height: 120dp; padding: 8dp; background-color: #cc4400; transform: rotate(20deg);">
<div id="child" style="display: block; width: 100dp; height: 60dp; padding: 8dp; background-color: #0088aa; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: translate(60dp, 0);"><p>Nested</p></div>
</div>
</body>
</rml>
)";

static const char* kInlineRotateRml = R"(
<rml><head><title>Rotate</title></head><body>
<div id="ref" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #0066cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin;"><p>Ref</p></div>
<div id="rotate" style="display: block; position: absolute; left: 240dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #00aa44; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: rotate(45deg);"><p>Rotate</p></div>
</body></rml>
)";

static const char* kInlineScaleRml = R"(
<rml><head><title>Scale</title></head><body>
<div id="ref" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #0066cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin;"><p>Ref</p></div>
<div id="scale" style="display: block; position: absolute; left: 240dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #ccaa00; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: scale(1.5);"><p>Scale</p></div>
</body></rml>
)";

static const char* kInlineTranslateRml = R"(
<rml><head><title>Translate</title></head><body>
<div id="ref" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #0066cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin;"><p>Ref</p></div>
<div id="translate" style="display: block; position: absolute; left: 240dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #8800cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: translate(80dp, 40dp);"><p>Move</p></div>
</body></rml>
)";

static const char* kInlineNestedRml = R"(
<rml><head><title>Nested</title></head><body>
<div id="ref" style="display: block; position: absolute; left: 40dp; top: 40dp; width: 120dp; height: 80dp; padding: 8dp; background-color: #0066cc; color: #ffffff; font-size: 18dp; font-family: LatoLatin;"><p>Ref</p></div>
<div id="parent" style="display: block; position: absolute; left: 40dp; top: 280dp; width: 160dp; height: 120dp; padding: 8dp; background-color: #cc4400; transform: rotate(20deg);">
<div id="child" style="display: block; width: 100dp; height: 60dp; padding: 8dp; background-color: #0088aa; color: #ffffff; font-size: 18dp; font-family: LatoLatin; transform: translate(60dp, 0);"><p>Nested</p></div>
</div>
</body></rml>
)";

static const char* InlineRmlForCase(TransformTestCase testCase)
{
    switch (testCase) {
        case TransformTestCase::Rotate:
            return kInlineRotateRml;
        case TransformTestCase::Scale:
            return kInlineScaleRml;
        case TransformTestCase::Translate:
            return kInlineTranslateRml;
        case TransformTestCase::Nested:
            return kInlineNestedRml;
        case TransformTestCase::All:
        default:
            return kInlineAllRml;
    }
}

static std::vector<const char*> RequiredElementIds(TransformTestCase testCase)
{
    switch (testCase) {
        case TransformTestCase::Rotate:
            return {"ref", "rotate"};
        case TransformTestCase::Scale:
            return {"ref", "scale"};
        case TransformTestCase::Translate:
            return {"ref", "translate"};
        case TransformTestCase::Nested:
            return {"ref", "parent", "child"};
        case TransformTestCase::All:
        default:
            return {"ref", "rotate", "scale", "translate", "parent", "child"};
    }
}

static Rml::ElementDocument* LoadTransformDocument(Rml::Context* rmlContext, TransformTestCase testCase)
{
    if (testCase == TransformTestCase::All) {
        if (Rml::ElementDocument* document = rmlContext->LoadDocument("phase2_transforms.rml")) {
            std::cout << "[OK] Loaded phase2_transforms.rml" << std::endl;
            return document;
        }
        std::cerr << "[WARN] phase2_transforms.rml failed, using inline RML" << std::endl;
    }

    const char* inlineRml = InlineRmlForCase(testCase);
    Rml::ElementDocument* document = rmlContext->LoadDocumentFromMemory(inlineRml, "phase2_transforms.rml");
    if (document) {
        std::cout << "[OK] Inline transform document loaded" << std::endl;
    }
    return document;
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

static TransformTestCase ParseTestCase(const std::string& value)
{
    if (value == "rotate") {
        return TransformTestCase::Rotate;
    }
    if (value == "scale") {
        return TransformTestCase::Scale;
    }
    if (value == "translate") {
        return TransformTestCase::Translate;
    }
    if (value == "nested") {
        return TransformTestCase::Nested;
    }
    return TransformTestCase::All;
}

static const char* TestCaseName(TransformTestCase testCase)
{
    switch (testCase) {
        case TransformTestCase::Rotate:
            return "rotate";
        case TransformTestCase::Scale:
            return "scale";
        case TransformTestCase::Translate:
            return "translate";
        case TransformTestCase::Nested:
            return "nested";
        case TransformTestCase::All:
        default:
            return "all";
    }
}

int main(int argc, char* argv[])
{
    int autoExitFrames = 0;
    TransformTestCase testCase = TransformTestCase::All;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) {
            autoExitFrames = std::atoi(argv[++i]);
        } else if (arg == "--case" && i + 1 < argc) {
            testCase = ParseTestCase(argv[++i]);
        }
    }

    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 2: CSS Transforms" << std::endl;
    std::cout << "  case: " << TestCaseName(testCase) << std::endl;
    std::cout << "============================================" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("RmlDiligent Phase 2", 1024, 768, SDL_WINDOW_RESIZABLE);
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

    Rml::ElementDocument* document = LoadTransformDocument(rmlContext, testCase);
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
    std::cout << "  - #ref: blue axis-aligned reference box (top-left)" << std::endl;
    if (testCase == TransformTestCase::All || testCase == TransformTestCase::Rotate) {
        std::cout << "  - #rotate: green box rotated ~45 deg (diamond-like)" << std::endl;
    }
    if (testCase == TransformTestCase::All || testCase == TransformTestCase::Scale) {
        std::cout << "  - #scale: yellow box larger than #ref" << std::endl;
    }
    if (testCase == TransformTestCase::All || testCase == TransformTestCase::Translate) {
        std::cout << "  - #translate: purple box offset down-right from layout position" << std::endl;
    }
    if (testCase == TransformTestCase::All || testCase == TransformTestCase::Nested) {
        std::cout << "  - #parent/#child: orange parent rotated, teal child translated inside" << std::endl;
    }
    std::cout << "\n[INFO] Starting render loop... Press ESC to exit (use --frames N for auto-exit)" << std::endl;

    bool running = true;
    int frameCount = 0;
    bool firstFrameSynced = false;
    uint32_t lastRmlDrawCount = 0;
    uint32_t lastTextureDrawCount = 0;
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
        lastTextureDrawCount = renderInterface.GetTextureDrawCount();

        if (frameCount == 0 || frameCount == 2) {
            std::cout << "[INFO] frame " << frameCount << " drawCount=" << lastRmlDrawCount
                      << " textureDraws=" << lastTextureDrawCount << std::endl;
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
    const bool textOk = lastTextureDrawCount > 0;

    std::cout << "\n============================================" << std::endl;
    if (elementsOk && renderOk && textOk) {
        std::cout << "  [PASS] Phase 2 通过!" << std::endl;
    } else {
        std::cout << "  [FAIL] Phase 2 验收未通过" << std::endl;
        if (!elementsOk) {
            std::cout << "  - 测试元素缺失或 layout 为 0 (" << elementsFound << "/" << requiredIds.size()
                      << " found, " << elementsWithLayout << " with layout)" << std::endl;
        }
        if (!renderOk) {
            std::cout << "  - drawCount=" << lastRmlDrawCount << " (need >= " << minDrawCount << ")" << std::endl;
        }
        if (!textOk) {
            std::cout << "  - textureDraws=0（文字 glyph 未渲染）" << std::endl;
        }
    }
    std::cout << "  case: " << TestCaseName(testCase) << std::endl;
    std::cout << "  帧数: " << frameCount << std::endl;
    std::cout << "  时间: " << elapsed << "s" << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "  last drawCount: " << lastRmlDrawCount << " textureDraws: " << lastTextureDrawCount << std::endl;
    std::cout << "============================================" << std::endl;

    document->Close();
    Rml::Shutdown();
    swapChain->Release();
    context->Release();
    device->Release();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return (elementsOk && renderOk && textOk) ? 0 : 1;
}
