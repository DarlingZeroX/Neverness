# Phase 4.3 — Replace NNRuntimeImGui With Official Diligent ImGui Backend

## 1. Analysis: Current NNRuntimeImGui Responsibilities

### Current Architecture
```
NNRuntimeImGui (SHARED library: NevernessRuntime-ImGui)
├── ImGui Core (imgui.cpp, imgui_draw, imgui_tables, imgui_widgets, imgui_demo)
├── ImGui FreeType (imgui_freetype.cpp — font rendering)
├── ImGui StackLayout (imgui_stacklayout — custom layout extension)
├── OpenGL Backend (imgui_impl_opengl3.cpp — MUST REMOVE)
├── SDL Renderer Backend (imgui_impl_sdlrenderer3.cpp — MUST REMOVE)
├── SDL3 Backend (imgui_impl_sdl3.cpp — KEEP, but replace with Diligent's version)
├── ImGuiEx (custom extensions: panels, windows, notifications, tasks)
├── ImGuiColorTextEdit (text editor widget)
├── imGuizmo (gizmo manipulation)
├── ImNodeEditor (node editor)
├── Imnodes (node editor alternative)
├── ImGuiLayer (SDL3Decorator, WindowDecorator — bridge to Neverness runtime)
└── OpenGL dependencies (opengl32.lib, imgui_impl_opengl3_loader.h)
```

### Responsibility Mapping
| Current | Action | New |
|---------|--------|-----|
| imgui_impl_opengl3.* | DELETE | Replaced by ImGuiDiligentRenderer |
| imgui_impl_opengl3_loader.h | DELETE | Not needed |
| imgui_impl_sdlrenderer3.* | DELETE | Not needed |
| imgui_impl_sdl3.* | REPLACE | Use Diligent's ImGuiImplSDL3 |
| imgui_impl_sdl3.h/cpp | REPLACE | Use Diligent's ImGuiImplSDL3 |
| ImGui Core | KEEP | Same files, no changes |
| ImGui FreeType | KEEP | Same files, no changes |
| ImGuiEx | KEEP | Same files, no changes |
| ImGuiColorTextEdit | KEEP | Same files, no changes |
| imGuizmo | KEEP | Same files, no changes |
| ImNodeEditor | KEEP | Same files, no changes |
| Imnodes | KEEP | Same files, no changes |
| SDL3Decorator | MODIFY | Replace OpenGL init with Diligent init |
| WindowDecorator | KEEP | Same interface, no changes |
| imconfig.h | KEEP | Same |

## 2. Files to DELETE (OpenGL Backend)
```
Include/Imgui/imgui_impl_opengl3.cpp
Include/Imgui/imgui_impl_opengl3.h
Include/Imgui/imgui_impl_opengl3_loader.h
Include/Imgui/imgui_impl_sdlrenderer3.cpp
Include/Imgui/imgui_impl_sdlrenderer3.h
```

## 3. Files to REPLACE (SDL3 Backend)
Instead of using the bundled `imgui_impl_sdl3.cpp`, we use Diligent's `ImGuiImplSDL3`:
- `DiligentTools/Imgui/interface/ImGuiImplSDL3.hpp`
- `DiligentTools/Imgui/src/ImGuiImplSDL3.cpp`
- `DiligentTools/Imgui/interface/ImGuiImplDiligent.hpp`
- `DiligentTools/Imgui/src/ImGuiImplDiligent.cpp`
- `DiligentTools/Imgui/interface/ImGuiDiligentRenderer.hpp`
- `DiligentTools/Imgui/src/ImGuiDiligentRenderer.cpp`

## 4. New Files to CREATE

### 4.1 NNDiligentImGuiRenderer.h (in Experiments module)
```cpp
#pragma once
// NNDiligentImGuiRenderer.h — Diligent ImGui backend adapter for Neverness

#include <NNRuntimeDiligent/NNDiligentConfig.h>
#include <SDL3/SDL.h>

// Forward declare Diligent types
namespace Diligent
{
    class ImGuiImplSDL3;
    struct IRenderDevice;
    struct IDeviceContext;
    struct ISwapChain;
}

namespace NNDiligent
{
    struct NNDiligentImGuiCreateInfo
    {
        ::Diligent::IRenderDevice*  pDevice     = nullptr;
        ::Diligent::IDeviceContext* pContext    = nullptr;
        ::Diligent::ISwapChain*     pSwapChain  = nullptr;
        SDL_Window*                 pWindow     = nullptr;
        bool                        EnableDocking = true;
    };

    class NNDiligentImGuiRenderer
    {
    public:
        NNDiligentImGuiRenderer() = default;
        ~NNDiligentImGuiRenderer();

        bool Initialize(const NNDiligentImGuiCreateInfo& info);
        void Shutdown();

        void NewFrame();
        void Render(::Diligent::IDeviceContext* pCtx);
        bool HandleSDLEvent(const SDL_Event* event);

    private:
        std::unique_ptr<::Diligent::ImGuiImplSDL3> m_Impl;
        SDL_Window* m_Window = nullptr;
    };
}
```

### 4.2 NNDiligentImGuiRenderer.cpp
```cpp
#include "NNDiligentImGuiRenderer.h"
#include <Imgui/interface/ImGuiImplSDL3.hpp>
#include <Imgui/interface/ImGuiImplDiligent.hpp>
#include <imgui/imgui.h>

namespace NNDiligent
{
    NNDiligentImGuiRenderer::~NNDiligentImGuiRenderer()
    {
        Shutdown();
    }

    bool NNDiligentImGuiRenderer::Initialize(const NNDiligentImGuiCreateInfo& info)
    {
        if (!info.pDevice || !info.pWindow) return false;
        m_Window = info.pWindow;

        // Enable docking if requested
        if (info.EnableDocking)
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Create Diligent ImGui implementation
        Diligent::ImGuiDiligentCreateInfo ci;
        ci.pDevice = info.pDevice;
        if (info.pSwapChain)
        {
            ci.BackBufferFmt = info.pSwapChain->GetDesc().ColorBufferFormat;
            ci.DepthBufferFmt = info.pSwapChain->GetDesc().DepthBufferFormat;
        }

        m_Impl = Diligent::ImGuiImplSDL3::Create(ci, info.pWindow);
        return m_Impl != nullptr;
    }

    void NNDiligentImGuiRenderer::Shutdown()
    {
        m_Impl.reset();
    }

    void NNDiligentImGuiRenderer::NewFrame()
    {
        if (!m_Impl || !m_Window) return;
        auto* sc = /* get swap chain */;
        auto w = sc->GetDesc().Width;
        auto h = sc->GetDesc().Height;
        m_Impl->NewFrame(w, h, Diligent::SURFACE_TRANSFORM_IDENTITY);
    }

    void NNDiligentImGuiRenderer::Render(::Diligent::IDeviceContext* pCtx)
    {
        if (m_Impl) m_Impl->Render(pCtx);
    }

    bool NNDiligentImGuiRenderer::HandleSDLEvent(const SDL_Event* event)
    {
        if (m_Impl) return m_Impl->HandleSDLEvent(event);
        return false;
    }
}
```

### 4.3 Test: TestImGuiDemo.cpp (in Experiments module)
```cpp
// Test that ImGui demo window renders through Diligent backend
#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNDiligentImGuiRenderer.h>
#include <imgui/imgui.h>

int main()
{
    // Create SDL window + Diligent device
    // Create ImGui renderer
    // Render loop: NewFrame → ShowDemoWindow → Render → Present
    // Print PASS after 10 frames
}
```

## 5. Implementation Steps

### Step 1: Create NNDiligentImGuiRenderer in Experiments module
- Create `NNRuntimeDiligent/ImGui/NNDiligentImGuiRenderer.h`
- Create `NNRuntimeDiligent/Source/ImGui/NNDiligentImGuiRenderer.cpp`
- Links against DiligentTools/Imgui (ImGuiImplSDL3 + ImGuiImplDiligent)

### Step 2: Create Test
- Create `NNRuntimeDiligent/Tests/TestImGuiDemo.cpp`
- Creates device, ImGui renderer, renders demo window for 10 frames

### Step 3: Update CMakeLists.txt
- Add ImGui source files from DiligentTools/Imgui
- Add NNDiligentImGuiRenderer to NNRuntimeDiligent
- Add test executable

### Step 4: DO NOT modify Runtime/NNRuntimeImGui yet
- The Experiments module provides the NEW implementation
- Runtime/NNRuntimeImGui can be migrated later (Phase 4.3b)
- This avoids breaking the existing engine build

## 6. Risk Analysis

| Risk | Mitigation |
|------|-----------|
| Diligent ImGui version mismatch | Use same ImGui version as bundled (check version) |
| SDL3 API differences | Diligent's ImGuiImplSDL3 supports SDL3 natively |
| Missing ImGui extensions (ImGuiEx, imGuizmo) | These are renderer-agnostic, work with any backend |
| Font atlas issues | Diligent's backend handles font texture creation |
| Docking support | ImGuiConfigFlags_DockingEnable in ImGui config |
| Multi-viewport | Not supported in initial implementation |

## 7. Build & Test

```powershell
cd E:\Neverness\Build\Experiments-Test
cmake E:\Neverness\Engine\Source\Experiments -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --target NNRuntimeDiligent --config Debug
cmake --build . --target NNImGuiTest --config Debug
```

## 8. Acceptance Criteria
- [ ] No OpenGL headers included anywhere
- [ ] No gl* API calls
- [ ] No dependency on NNRuntimeRHI/OpenGL
- [ ] ImGui demo window renders correctly via Diligent
- [ ] SDL3 input works (keyboard, mouse)
- [ ] Docking works
- [ ] Compilation passes for Vulkan backend
