// =============================================================================
// TestSamplesSmoke.cpp
// Phase 7: 官方 Samples 冒烟 — 使用 RmlDiligent Backend 加载 assets/demo.rml 并渲染数帧。
// =============================================================================

#include <Shell.h>
#include <RmlUi_Backend.h>
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

#include "RmlDiligentRenderInterface.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
    int frames = 3;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--frames" && i + 1 < argc) {
            frames = std::max(1, std::atoi(argv[++i]));
        }
    }

    if (!Shell::Initialize()) {
        std::cerr << "[FAIL] Shell::Initialize (Samples root not found — copy Samples/ next to exe)" << std::endl;
        return 1;
    }

    if (!Backend::Initialize("RmlDiligent Samples Smoke", 1024, 768, false)) {
        std::cerr << "[FAIL] Backend::Initialize" << std::endl;
        Shell::Shutdown();
        return 1;
    }

    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    if (!Rml::Initialise()) {
        std::cerr << "[FAIL] Rml::Initialise" << std::endl;
        Backend::Shutdown();
        Shell::Shutdown();
        return 1;
    }

    Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(1024, 768));
    if (!context) {
        std::cerr << "[FAIL] CreateContext" << std::endl;
        Rml::Shutdown();
        Backend::Shutdown();
        Shell::Shutdown();
        return 1;
    }

    Rml::Debugger::Initialise(context);
    Shell::LoadFonts();

    Rml::ElementDocument* document = context->LoadDocument("assets/demo.rml");
    if (!document) {
        std::cerr << "[FAIL] LoadDocument(assets/demo.rml)" << std::endl;
        Rml::Shutdown();
        Backend::Shutdown();
        Shell::Shutdown();
        return 1;
    }
    document->Show();

    auto* render = static_cast<RmlDiligent::RmlDiligentRenderInterface*>(Backend::GetRenderInterface());

    for (int f = 0; f < frames; ++f) {
        if (!Backend::ProcessEvents(context, &Shell::ProcessKeyDownShortcuts, true)) {
            break;
        }
        context->Update();
        Backend::BeginFrame();
        context->Render();
        Backend::PresentFrame();
    }

    const uint32_t draws = render ? render->GetDrawCount() : 0;
    const bool pass = draws > 0;

    std::cout << "[INFO] frames=" << frames << " drawCount=" << draws << std::endl;
    if (pass) {
        std::cout << "[PASS] Phase 7 Samples smoke" << std::endl;
    } else {
        std::cerr << "[FAIL] drawCount=0" << std::endl;
    }

    Rml::Shutdown();
    Backend::Shutdown();
    Shell::Shutdown();
    return pass ? 0 : 1;
}
