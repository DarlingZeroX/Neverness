// =============================================================================
// TestPerformance.cpp
// Phase 6: SRB 缓存性能基准 — 采集 FPS、帧时间分位、DrawCall/SRB 命中与 MemoryStats。
// 通过条件（实验阶段）：
//   - SRB 缓存开启：warmup 后 hits/(hits+misses) > 80%（phase5_filters.rml）
//   - 可选 --compare-ab：缓存 FPS ≥ 无缓存 FPS × 1.15
// 注：仓库无独立 DX12 Backend 对比程序，A/B 为自研 SRB 开关对比。
// =============================================================================

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

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

struct BenchmarkConfig {
    int frames = 300;
    int warmup = 30;
    bool srbCacheEnabled = true;
    bool compareAb = false;
    bool leakCheck = false;
    std::string docPath = "phase5_filters.rml";
};

struct BenchmarkResult {
    float fps = 0.0f;
    float p50Ms = 0.0f;
    float p95Ms = 0.0f;
    uint32_t drawCount = 0;
    uint32_t textureDraws = 0;
    uint32_t filterRenders = 0;
    uint32_t composites = 0;
    uint32_t srbHits = 0;
    uint32_t srbMisses = 0;
    size_t srbEntries = 0;
    RmlDiligent::MemoryStats memory{};
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

    Rml::FileHandle Open(const Rml::String& path) override
    {
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
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
    {
        return fread(buffer, 1, size, (FILE*)file);
    }
    bool Seek(Rml::FileHandle file, long offset, int origin) override
    {
        return fseek((FILE*)file, offset, origin) == 0;
    }
    size_t Tell(Rml::FileHandle file) override { return ftell((FILE*)file); }

private:
    Rml::String root;
};

class RmlDiligentSystemInterface : public Rml::SystemInterface {
public:
    double GetElapsedTime() override { return SDL_GetTicks() / 1000.0; }

    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override
    {
        if (type == Rml::Log::Type::LT_ERROR) {
            std::cerr << "[RmlUi ERROR] " << message.c_str() << std::endl;
        }
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
        engineCI.EnableValidation = false;
        factory->CreateDeviceAndContextsD3D12(engineCI, ppDevice, ppContext);
        if (*ppDevice) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D12(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
            if (*ppSwapChain) {
                std::cout << "[Phase6] Backend: D3D12" << std::endl;
                return true;
            }
        }
    }
#endif

#ifdef D3D11_SUPPORTED
    if (auto* factory = Diligent::GetEngineFactoryD3D11()) {
        Diligent::EngineD3D11CreateInfo engineCI;
        engineCI.EnableValidation = false;
        factory->CreateDeviceAndContextsD3D11(engineCI, ppDevice, ppContext);
        if (*ppDevice) {
            Diligent::Win32NativeWindow nativeWnd{hwnd};
            factory->CreateSwapChainD3D11(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
            if (*ppSwapChain) {
                std::cout << "[Phase6] Backend: D3D11" << std::endl;
                return true;
            }
        }
    }
#endif

    return false;
}

static float Percentile(std::vector<float> values, float p)
{
    if (values.empty()) {
        return 0.0f;
    }
    std::sort(values.begin(), values.end());
    const float rank = p * static_cast<float>(values.size() - 1);
    const size_t lo = static_cast<size_t>(rank);
    const size_t hi = std::min(lo + 1, values.size() - 1);
    const float frac = rank - static_cast<float>(lo);
    return values[lo] * (1.0f - frac) + values[hi] * frac;
}

static BenchmarkResult RunBenchmark(
    Rml::Context* rmlContext,
    RmlDiligent::RmlDiligentRenderInterface& renderInterface,
    Diligent::ISwapChain* swapChain,
    int warmupFrames,
    int measureFrames)
{
    BenchmarkResult result{};
    std::vector<float> frameTimesMs;
    frameTimesMs.reserve(static_cast<size_t>(measureFrames));

    const int totalFrames = warmupFrames + measureFrames;
    for (int frame = 0; frame < totalFrames; ++frame) {
        if (frame == warmupFrames) {
            renderInterface.ResetPerfStats();
        }

        const Uint64 frameStart = SDL_GetTicks();

        SDL_PumpEvents();
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT) {
                break;
            }
        }

        rmlContext->Update();
        renderInterface.BeginFrame();
        rmlContext->Render();
        renderInterface.EndFrame();
        swapChain->Present(0);

        const float frameMs = static_cast<float>(SDL_GetTicks() - frameStart);
        if (frame >= warmupFrames) {
            frameTimesMs.push_back(frameMs);
        }
    }

    result.drawCount = renderInterface.GetDrawCount();
    result.textureDraws = renderInterface.GetTextureDrawCount();
    result.filterRenders = renderInterface.GetFilterRenderCount();
    result.composites = renderInterface.GetCompositeCount();
    result.srbHits = renderInterface.GetSrbCacheHits();
    result.srbMisses = renderInterface.GetSrbCacheMisses();
    result.srbEntries = renderInterface.GetSrbCacheEntryCount();
    result.memory = renderInterface.GetMemoryStats();

    if (!frameTimesMs.empty()) {
        const float totalMs = std::accumulate(frameTimesMs.begin(), frameTimesMs.end(), 0.0f);
        result.fps = static_cast<float>(frameTimesMs.size()) / (totalMs / 1000.0f);
        result.p50Ms = Percentile(frameTimesMs, 0.50f);
        result.p95Ms = Percentile(frameTimesMs, 0.95f);
    }

    return result;
}

static void PrintResult(const char* label, const BenchmarkResult& r)
{
    const uint32_t srbTotal = r.srbHits + r.srbMisses;
    const float hitRate = srbTotal > 0 ? (100.0f * static_cast<float>(r.srbHits) / static_cast<float>(srbTotal)) : 0.0f;

    std::cout << "\n--- " << label << " ---" << std::endl;
    std::cout << "  FPS: " << r.fps << "  p50: " << r.p50Ms << "ms  p95: " << r.p95Ms << "ms" << std::endl;
    std::cout << "  drawCount=" << r.drawCount << " textureDraws=" << r.textureDraws
              << " filterRenders=" << r.filterRenders << " composites=" << r.composites << std::endl;
    std::cout << "  srbHits=" << r.srbHits << " srbMisses=" << r.srbMisses
              << " hitRate=" << hitRate << "% entries=" << r.srbEntries << std::endl;
    std::cout << "  memory: rtFree=" << r.memory.pooledRtFreeCount
              << " rtActive=" << r.memory.pooledRtActiveLayers
              << " srbCache=" << r.memory.srbCacheEntries
              << " texSrb=" << r.memory.textureSrbEntries << std::endl;
}

static BenchmarkConfig ParseArgs(int argc, char* argv[])
{
    BenchmarkConfig cfg{};
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--no-srb-cache") {
            cfg.srbCacheEnabled = false;
        } else if (arg == "--compare-ab") {
            cfg.compareAb = true;
        } else if (arg == "--leak-check") {
            cfg.leakCheck = true;
        } else if (arg == "--frames" && i + 1 < argc) {
            cfg.frames = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--warmup" && i + 1 < argc) {
            cfg.warmup = std::max(0, std::atoi(argv[++i]));
        } else if (arg == "--doc" && i + 1 < argc) {
            cfg.docPath = argv[++i];
        }
    }
    return cfg;
}

int main(int argc, char* argv[])
{
    const BenchmarkConfig cfg = ParseArgs(argc, argv);

    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 6 Performance Benchmark" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "  frames=" << cfg.frames << " warmup=" << cfg.warmup
              << " srbCache=" << (cfg.srbCacheEnabled ? "on" : "off")
              << " doc=" << cfg.docPath << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[FAIL] SDL_Init: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("RmlDiligent Phase6", 1024, 768, SDL_WINDOW_HIDDEN);
    if (!window) {
        std::cerr << "[FAIL] SDL_CreateWindow" << std::endl;
        SDL_Quit();
        return 1;
    }

    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> swapChain;
    if (!CreateDeviceAndSwapChain(window, &device, &context, &swapChain)) {
        std::cerr << "[FAIL] Device/SwapChain creation failed" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    RmlDiligent::RmlDiligentRenderInterface renderInterface;
    if (!renderInterface.Initialize(device, context, swapChain)) {
        std::cerr << "[FAIL] RenderInterface init failed" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const std::string samplesRoot = ResolveRmlUiSamplesRoot(argc > 0 ? argv[0] : nullptr);
    RmlDiligentFileInterface fileInterface(samplesRoot);
    RmlDiligentSystemInterface systemInterface;

    Rml::SetSystemInterface(&systemInterface);
    Rml::SetFileInterface(&fileInterface);
    Rml::SetRenderInterface(&renderInterface);
    if (!Rml::Initialise()) {
        std::cerr << "[FAIL] Rml::Initialise failed" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int width = 1024;
    int height = 768;
    SDL_GetWindowSize(window, &width, &height);
    Rml::Context* rmlContext = Rml::CreateContext("main", Rml::Vector2i(width, height), &renderInterface);
    if (!rmlContext) {
        std::cerr << "[FAIL] RmlUi Context creation failed" << std::endl;
        Rml::Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Rml::LoadFontFace("assets/LatoLatin-Regular.ttf", true);
    renderInterface.SetProjectionMatrix(width, height);

    Rml::ElementDocument* document = rmlContext->LoadDocument(cfg.docPath.c_str());
    if (!document) {
        std::cerr << "[WARN] " << cfg.docPath << " failed, trying phase5_filters.rml" << std::endl;
        document = rmlContext->LoadDocument("phase5_filters.rml");
    }
    if (!document) {
        std::cerr << "[FAIL] Document load failed" << std::endl;
        Rml::Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    document->Show();

    for (int i = 0; i < 4; ++i) {
        rmlContext->Update();
    }

    auto runOnce = [&](bool cacheEnabled) -> BenchmarkResult {
        renderInterface.SetSrbCacheEnabled(cacheEnabled);
        renderInterface.ResetPerfStats();
        return RunBenchmark(rmlContext, renderInterface, swapChain, cfg.warmup, cfg.frames);
    };

    BenchmarkResult cachedResult{};
    BenchmarkResult uncachedResult{};

    if (cfg.compareAb) {
        uncachedResult = runOnce(false);
        PrintResult("SRB cache OFF", uncachedResult);
        cachedResult = runOnce(true);
        PrintResult("SRB cache ON", cachedResult);
    } else {
        cachedResult = runOnce(cfg.srbCacheEnabled);
        PrintResult(cfg.srbCacheEnabled ? "SRB cache ON" : "SRB cache OFF", cachedResult);
    }

    if (cfg.leakCheck) {
        const size_t entriesBefore = renderInterface.GetSrbCacheEntryCount();
        document->Close();
        rmlContext->UnloadAllDocuments();
        for (int i = 0; i < 2; ++i) {
            rmlContext->Update();
        }
        if (Rml::ElementDocument* reloaded = rmlContext->LoadDocument(cfg.docPath.c_str())) {
            reloaded->Show();
            for (int i = 0; i < 4; ++i) {
                rmlContext->Update();
            }
            runOnce(cfg.srbCacheEnabled);
        }
        const size_t entriesAfter = renderInterface.GetSrbCacheEntryCount();
        std::cout << "\n[leak-check] srbEntries before=" << entriesBefore << " after=" << entriesAfter << std::endl;
    }

    const uint32_t srbTotal = cachedResult.srbHits + cachedResult.srbMisses;
    const float hitRate = srbTotal > 0 ? (100.0f * static_cast<float>(cachedResult.srbHits) / static_cast<float>(srbTotal)) : 0.0f;

    bool pass = true;
    if (cfg.srbCacheEnabled || cfg.compareAb) {
        if (hitRate < 80.0f) {
            std::cout << "[FAIL] SRB hit rate " << hitRate << "% < 80%" << std::endl;
            pass = false;
        }
        if (cachedResult.filterRenders < 6) {
            std::cout << "[FAIL] filterRenders=" << cachedResult.filterRenders << " < 6" << std::endl;
            pass = false;
        }
    }

    if (cfg.compareAb && uncachedResult.fps > 0.0f) {
        const float requiredFps = uncachedResult.fps * 1.15f;
        if (cachedResult.fps < requiredFps) {
            std::cout << "[FAIL] Cached FPS " << cachedResult.fps << " < uncached*1.15 (" << requiredFps << ")" << std::endl;
            pass = false;
        } else {
            std::cout << "[OK] Cached FPS gain: " << (cachedResult.fps / uncachedResult.fps) << "x" << std::endl;
        }
    }

    std::cout << "\n============================================" << std::endl;
    if (pass) {
        std::cout << "  [PASS] Phase 6 benchmark" << std::endl;
    } else {
        std::cout << "  [FAIL] Phase 6 benchmark" << std::endl;
    }
    std::cout << "============================================" << std::endl;

    Rml::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return pass ? 0 : 1;
}
