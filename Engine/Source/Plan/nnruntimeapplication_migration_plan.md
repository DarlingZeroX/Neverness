# NNRuntimeApplication C++ → C# 迁移计划 (v2)

> **目标**: 将 `NNRuntimeApplication` (C++ SDL3 静态库) 迁移到 `Neverness.Runtime.Application` (C# managed 模块)，使用 ppy/SDL3-CS + ImGui.NET 替代 C++ SDL3/ImGui 调用。
>
> **创建日期**: 2026-06-23
> **修订日期**: 2026-06-23 (v2 - 架构审查修订)
> **状态**: 待审查

---

## 1. 迁移范围总览

### 1.1 从 C++ 迁移到 C# 的功能

| 功能 | C++ 原文件 | C# 新实现 |
|------|-----------|----------|
| SDL3 生命周期 | `RuntimeApplication.cpp` (Init/Quit) | `SdlApplicationHost.cs` |
| 事件泵 | `RuntimeApplication.cpp` (PumpEvents) + `EventQueue.h` + `SDL3EventTranslator.h` | `SdlEventBridge.cs` |
| 窗口管理 | `Window.h/cpp` + `WindowRegistry.h/cpp` | `SdlWindowManager.cs` + `SdlWindow.cs` |
| VFS 初始化 | `EditorInitializer.h/cpp` | `EditorVfsInitializer.cs` |
| ImGui 层 | `ImGuiLayer.h/cpp` | `ImGui.NET` (完全托管) |
| ABI 函数表导出 | `BuildApplicationApi.cpp` + `BuildWindowApi.cpp` + `BuildEventApi.cpp` | **删除** (不再需要) |

### 1.2 保留在 C++ 的功能

| 功能 | C++ 文件 | 说明 |
|------|---------|------|
| ImGui SDL3 Backend | 新建 `ImGuiSDL3Backend.h/cpp` | 仅封装 `ImGui_ImplSDL3_*` 函数 |
| ImGui Diligent Backend | 新建 `ImGuiDiligentBackend.h/cpp` | 仅封装 `ImGui_ImplDiligent_*` 函数 |
| Diligent 设备 | 现有 `DiligentRuntimeApi` | C# 端通过 DiligentEngine.NET 创建 |

### 1.3 最终架构

```
C# Managed Host (Neverness.Runtime.Application)
  ├─ SDL3-CS: SDL_Init / SDL_PollEvent / SDL_Quit
  ├─ SdlWindowManager: 窗口创建/管理 (SDL3-CS)
  ├─ SdlEventBridge: SDL_Event → C# 事件分发
  ├─ EditorVfsInitializer: VFS 挂载 (通过 NNVfsApi)
  ├─ ImGui.NET: ImGui UI 全部托管
  └─ DiligentEngine.NET: 设备创建

Native (C++)
  ├─ Diligent Engine (保留)
  ├─ ImGuiSDL3Backend (极小封装)
  └─ ImGuiDiligentBackend (极小封装)
```

### 1.4 标记为 Legacy 的 C++ 模块

- `NNDesktopApplication` — Editor 旧 C++ 入口
- `NNLauncher` — 项目启动器
- `NNNodeGraphApp` — 节点图编辑器
- `NNPackageTool` — 打包工具

这些模块依赖 `NNRuntimeApplication` 的窗口和 ImGui，迁移后不再维护。

---

## 2. 架构变更

### 2.1 迁移前 (C++ 驱动)

```
C++ Native Host
  ├─ SDL_Init / SDL_PollEvent / SDL_Quit
  ├─ WindowRegistry → VGWindow (SDL3 + Diligent D3D12)
  ├─ EventQueue (SPSC) → C# 读取
  ├─ ImGuiLayer (ImGuiImplSDL3 + Diligent)
  └─ Build*Api() → NNNativeEngineApi 函数指针表
       └─ C# ApplicationHost/WindowHost 调用
```

### 2.2 迁移后 (C# 驱动)

```
C# Managed Host
  ├─ SDL3-CS
  │    └─ SDL_Init / SDL_PollEvent / SDL_Quit
  ├─ SdlWindowManager
  │    └─ 窗口创建/销毁 (SDL_Window*)
  ├─ SdlEventBridge
  │    └─ SDL_Event 直接分发 (不翻译)
  ├─ RenderSurfaceHost
  │    ├─ CreateSurface(SDL_Window*)
  │    ├─ CreateSwapChain(surface)
  │    └─ Resize(surface)
  ├─ ImGui.NET
  │    └─ 完全托管的 ImGui UI
  └─ C++ ImGui Backend (极小 ABI)
       ├─ nn_imgui_backend_initialize(SDL_Window*, IDevice*, IDeviceContext*, ISwapChain*)
       ├─ nn_imgui_backend_shutdown()
       ├─ nn_imgui_backend_new_frame()
       ├─ nn_imgui_backend_render()
       └─ nn_imgui_backend_process_event(SDL_Event*)
```

### 2.3 NNNativeEngineApi 变更

**移除的子表** (不再由 C++ 提供):
- `NNApplicationApi` — SDL 生命周期迁移到 C#
- `NNWindowApi` — 窗口管理迁移到 C#
- `NNEventApi` — 事件系统迁移到 C#

**保留的子表** (仍由 C++ 提供):
- `NNRenderApi`, `NNRenderAssetApi`, `NNViewportRenderApi`
- `NNAudioApi`, `NNInputApi`, `NNAsyncWaitApi`
- `NNVfsApi`, `NNDiligentApi`

**新增的子表**:
- `NNImGuiBackendApi` — 极小的 ImGui Backend 封装 (5个函数)

**不新增**:
- ~~`NNImGuiApi`~~ — 不需要，ImGui UI 全部在 C# 用 ImGui.NET

---

## 3. 关键设计决策

### 3.1 ImGui: ImGui.NET 替代 C++ ImGuiLayer

**决策**: 不保留 C++ ImGuiLayer，改用 ImGui.NET 完全托管。

**理由**:
- 避免新增 `NNImGuiApi` ABI 子表 (与删除 `NNApplicationApi` 等的目标矛盾)
- ImGui.NET 是 ImGui 的官方 C# 绑定，API 完整
- Editor UI 全部托管，职责清晰

**C++ 只保留**:
```cpp
// ImGuiSDL3Backend.h — 极小封装
extern "C" {
    NN_API bool nn_imgui_backend_sdl3_init(SDL_Window* window);
    NN_API void nn_imgui_backend_sdl3_shutdown();
    NN_API void nn_imgui_backend_sdl3_new_frame();
    NN_API void nn_imgui_backend_sdl3_process_event(const SDL_Event* event);
}

// ImGuiDiligentBackend.h — 极小封装
extern "C" {
    NN_API bool nn_imgui_backend_diligent_init(void* device, void* context, void* swapChain);
    NN_API void nn_imgui_backend_diligent_shutdown();
    NN_API void nn_imgui_backend_diligent_new_frame();
    NN_API void nn_imgui_backend_diligent_render(ImDrawData* drawData);
}
```

**C# 端**:
```csharp
// ImGuiBackendBridge.cs — 调用 C++ backend
internal static unsafe class ImGuiBackendBridge
{
    public static void Initialize(SDL_Window* window, IntPtr device, IntPtr context, IntPtr swapChain)
    {
        nn_imgui_backend_sdl3_init(window);
        nn_imgui_backend_diligent_init(device, context, swapChain);
    }

    public static void NewFrame()
    {
        nn_imgui_backend_sdl3_new_frame();
        nn_imgui_backend_diligent_new_frame();
    }

    public static void Render()
    {
        ImGui.Render();
        nn_imgui_backend_diligent_render(ImGui.GetDrawData());
    }

    public static void ProcessEvent(SDL_Event* e)
    {
        nn_imgui_backend_sdl3_process_event(e);
    }
}
```

### 3.2 Window: SDL_WindowID 直接作为 Handle

**决策**: 不自增 `NNWindowHandle`，直接用 SDL3 的 `SDL_WindowID` (uint32_t)。

**理由**:
- SDL3 已有唯一标识: `SDL_GetWindowID()` 返回 `uint32_t`
- 无需维护 `NNWindowHandle → SDL_WindowID` 双重映射
- 减少复杂度

**实现**:
```csharp
public readonly struct WindowHandle : IEquatable<WindowHandle>
{
    public readonly uint Value;
    public WindowHandle(uint value) => Value = value;
    public bool Equals(WindowHandle other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is WindowHandle h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public static bool operator ==(WindowHandle left, WindowHandle right) => left.Equals(right);
    public static bool operator !=(WindowHandle left, WindowHandle right) => !left.Equals(right);
}

internal static unsafe class SdlWindowManager
{
    private static readonly Dictionary<WindowHandle, SdlWindow> s_windows = new();

    public static WindowHandle Create(string title, int width, int height, ...)
    {
        var sdlWindow = SDL3.SDL_CreateWindow(...);
        uint id = SDL3.SDL_GetWindowID(sdlWindow);
        var handle = new WindowHandle(id);
        s_windows[handle] = new SdlWindow(sdlWindow, handle);
        return handle;
    }
}
```

### 3.3 Event: SDL_Event 直接分发，不翻译

**决策**: 删除 `TranslateEvent()`，直接分发 `SDL_Event`。

**理由**:
- 已决定删除 `NNEvent` 128字节 POD
- 不需要兼容旧 `NNEvent` 结构
- C# 代码可以直接用 `SDL_Event.Type` 判断

**实现**:
```csharp
internal static unsafe class SdlEventBridge
{
    // 直接分发 SDL_Event，不翻译
    public static event Action<SDL_Event>* OnEvent;

    // 便捷事件 (从 SDL_Event 中提取常用信息)
    public static event Action<uint, string>? OnDropFile;   // windowId, filePath
    public static event Action<uint, string>? OnDropText;   // windowId, text
    public static event Action<uint, string>? OnTextInput;  // windowId, text

    public static bool PumpEvents()
    {
        SDL_Event e;
        while (SDL3.SDL_PollEvent(&e))
        {
            // 直接分发原始事件
            OnEvent?.Invoke(&e);

            // 提取常用事件到便捷回调
            switch (e.Type)
            {
                case SDL_EventType.SDL_EVENT_DROP_FILE:
                    OnDropFile?.Invoke(e.drop.WindowID, Marshal.PtrToStringUTF8((nint)e.drop.Data)!);
                    break;
                case SDL_EventType.SDL_EVENT_TEXT_INPUT:
                    OnTextInput?.Invoke(e.text.WindowID, Marshal.PtrToStringUTF8((nint)e.text.Text)!);
                    break;
                case SDL_EventType.SDL_EVENT_QUIT:
                    s_shouldQuit = true;
                    break;
            }

            // 传给 ImGui backend
            ImGuiBackendBridge.ProcessEvent(&e);
        }
        return !s_shouldQuit;
    }
}
```

### 3.4 Diligent: RenderSurfaceHost 分离

**决策**: 不在 `SdlWindow` 里创建 Diligent，拆分出 `RenderSurfaceHost`。

**理由**:
- 未来多窗口编辑器 (Main Window, Game Window, Preview Window, Asset Preview)
- 每个窗口是独立 Surface
- 类似 Unity: Window → Camera → RenderTarget 分离

**实现**:
```csharp
public sealed class RenderSurface : IDisposable
{
    public IntPtr Surface { get; }      // Diligent ISurface*
    public IntPtr SwapChain { get; }    // Diligent ISwapChain*
    public WindowHandle WindowHandle { get; }

    public void Resize(int width, int height) { ... }
    public void Dispose() { ... }
}

internal static class RenderSurfaceHost
{
    private static readonly Dictionary<WindowHandle, RenderSurface> s_surfaces = new();

    public static RenderSurface CreateSurface(WindowHandle windowHandle)
    {
        var window = SdlWindowManager.Resolve(windowHandle);
        nint hWnd = window.GetPlatformHandle(); // HWND on Windows

        // 通过 DiligentEngine.NET 创建
        var surface = DiligentApi.CreateSurface(hWnd);
        var swapChain = DiligentApi.CreateSwapChain(surface);

        var renderSurface = new RenderSurface(surface, swapChain, windowHandle);
        s_surfaces[windowHandle] = renderSurface;
        return renderSurface;
    }

    public static void Resize(WindowHandle handle, int width, int height)
    {
        if (s_surfaces.TryGetValue(handle, out var surface))
            surface.Resize(width, height);
    }

    public static void Destroy(WindowHandle handle)
    {
        if (s_surfaces.Remove(handle, out var surface))
            surface.Dispose();
    }
}
```

### 3.5 VFS: 初始化顺序提前

**决策**: VFS 在 SDL 之前初始化。

**理由**:
- Config、ProjectSettings、EngineSettings、Localization、AssetDatabase 都访问 VFS
- SDL 不依赖 VFS

**顺序**:
```csharp
public static bool Initialize()
{
    // 1. Native API Bootstrap (加载 DLL)
    NativeApiBootstrap.Initialize();

    // 2. 发现项目根目录
    var projectRoot = DiscoverProjectRoot();

    // 3. VFS 初始化 (在 SDL 之前!)
    EditorVfsInitializer.InitializeVfs(projectRoot);

    // 4. SDL 初始化
    SdlApplicationHost.Initialize();

    return true;
}
```

---

## 4. Phase 分解 (风险优先)

### Phase 0: SDL3-CS 验证 (1天)

**目标**: 验证 SDL3-CS NuGet 包可用性

1. 在 `Neverness.Runtime.Application.csproj` 中添加 NuGet 包:
   ```xml
   <PackageReference Include="SDL3-CS" Version="*" />
   ```
   > ⚠️ **待确认**: NuGet 包的确切名称和最新版本号需要验证。

2. 创建 `SdlApplicationHost.cs` 骨架:
   ```csharp
   using SDL;
   public static unsafe class SdlApplicationHost
   {
       public static bool Initialize()
       {
           return SDL3.SDL_Init(SDL_InitFlags.SDL_INIT_VIDEO |
                                SDL_InitFlags.SDL_INIT_EVENTS |
                                SDL_InitFlags.SDL_INIT_AUDIO);
       }
       public static void Shutdown() => SDL3.SDL_Quit();
   }
   ```

3. 验证:
   - SDL3-CS NuGet 包包含 Windows x64 native 二进制
   - `SDL_Init()` 成功调用
   - `SDL_Window*` 可以通过 `SDL_CreateWindow()` 获取

**产出**: SDL3-CS 可用，基础 Init/Quit/Window 调用通过

---

### Phase 1: DiligentEngine.NET + ImGui Backend PoC (2天)

**目标**: 验证 Diligent.NET 设备能否被 C++ ImGui Backend 使用

**这是整个计划最大的风险点，必须最先验证。**

1. 验证 DiligentEngine.NET 的 `IDevice` / `IDeviceContext` / `ISwapChain` 是否为 COM 指针包装:
   ```csharp
   // 检查 DiligentEngine.NET 源码
   // 确认 IDevice 内部是 IntPtr NativePointer
   // 而不是 Managed Wrapper / RCW / Runtime Proxy
   ```

2. 创建测试:
   ```csharp
   // C# 端创建设备
   GraphicsDevice.Initialize(GraphicsBackend.D3D12, ...);
   var device = GraphicsDevice.Instance.Device;      // IDevice
   var context = GraphicsDevice.Instance.Context;      // IDeviceContext
   var swapChain = GraphicsDevice.Instance.SwapChain;  // ISwapChain

   // 获取原生指针
   IntPtr devicePtr = device.NativePointer;
   IntPtr contextPtr = context.NativePointer;
   IntPtr swapChainPtr = swapChain.NativePointer;

   // 传给 C++ ImGui Backend
   nn_imgui_backend_diligent_init(devicePtr, contextPtr, swapChainPtr);
   ```

3. 验证:
   - C++ ImGui Backend 能正确使用 C# 传入的 Diligent 设备指针
   - ImGui 渲染输出正确
   - 设备生命周期无冲突

**如果验证失败**: 回退到 C++ 端创建 Diligent 设备，C# 只获取引用。

**产出**: 确认 Diligent.NET + C++ ImGui Backend 兼容性

---

### Phase 2: 窗口管理 (2天)

**目标**: C# 端实现 SDL 窗口创建和管理

**文件清单**:
- `Private/SdlWindowManager.cs` — 窗口注册表
- `Public/SdlWindow.cs` — 窗口包装类
- `Public/WindowHandle.cs` — WindowHandle 结构 (使用 SDL_WindowID)

**关键实现**:
- `Create` 内部调用 `SDL3.SDL_CreateWindow()` → 获取 `SDL_Window*`
- `GetPlatformHandle` 通过 `SDL3.SDL_GetPointerProperty(SDL_WINDOW_PROPERTY_HANDLE)` 获取 HWND
- 第一个创建的窗口自动成为 Primary
- WindowHandle 直接使用 `SDL_GetWindowID()` 返回的 uint32_t

**产出**: C# 可创建/管理 SDL 窗口，获取原生句柄

---

### Phase 3: 事件系统 (1天)

**目标**: C# 端实现 SDL 事件泵和事件分发

**文件清单**:
- `Private/SdlEventBridge.cs` — SDL 事件泵 + 直接分发

**关键变更**:
- **删除 `TranslateEvent()`** — 直接分发 `SDL_Event`
- **不再需要 SPSC EventQueue** — C# 直接处理事件
- **不再需要 NNEvent 128字节 POD** — C# 用 `SDL_Event` 原生结构
- **提供便捷回调** — `OnDropFile`, `OnTextInput` 等从 `SDL_Event` 中提取

**产出**: C# 完全控制事件循环，事件可分发到 C# 和 ImGui Backend

---

### Phase 4: VFS 初始化 (1天)

**目标**: 将 EditorInitializer 的 VFS 挂载逻辑迁移到 C#

**文件清单**:
- 新建 `Private/EditorVfsInitializer.cs`
- 删除 C++ `EditorInitializer.h/cpp`

**关键点**:
- 使用 `NNVfsApi.AddFileSystem(alias, type, root)` 挂载
- VFS 初始化在 SDL 之前执行
- C++ `EditorInitializer::PakResource()` 功能需要确认是否已有 C# 等价实现
- `pfd::message` 对话框用 Avalonia 对话框替代

**产出**: VFS 初始化完全在 C# 端完成

---

### Phase 5: RenderSurface 分离 (1天)

**目标**: 实现 RenderSurfaceHost，分离 Window 和 SwapChain

**文件清单**:
- `Private/RenderSurfaceHost.cs` — Surface/SwapChain 管理
- `Public/RenderSurface.cs` — Surface 包装类

**关键点**:
- 每个 Window 可以有独立的 RenderSurface
- SwapChain 创建通过 DiligentEngine.NET
- 支持 Resize

**产出**: Window 和 RenderTarget 分离，支持多窗口

---

### Phase 6: ImGui Backend 封装 (2天)

**目标**: C++ ImGui Backend 极小封装 + C# ImGuiBackendBridge

**C++ 文件**:
- 新建 `Private/ImGuiSDL3Backend.cpp` — 封装 `ImGui_ImplSDL3_*`
- 新建 `Private/ImGuiDiligentBackend.cpp` — 封装 `ImGui_ImplDiligent_*`
- 新建 `Include/ImGuiBackendApi.h` — 导出函数声明

**C# 文件**:
- `Private/ImGuiBackendBridge.cs` — 调用 C++ backend

**关键点**:
- C++ 只封装 backend 函数，不管理 ImGui 生命周期
- ImGui 生命周期 (Init/Shutdown/NewFrame/Render) 由 ImGui.NET 管理
- Backend 初始化需要 SDL_Window* + Diligent 设备指针

**产出**: ImGui Backend 可用，C# 通过 ImGui.NET + Backend Bridge 驱动 ImGui

---

### Phase 7: 生命周期整合 (1天)

**目标**: 整合所有组件，实现完整的 C# 应用启动链

**修改文件**:
- `ApplicationHost.cs` — 重写为 SDL3 驱动的生命周期
- `Neverness.Runtime.Bootstrap/RuntimeBootstrap.cs` — 适配新的 ApplicationHost

**ApplicationHost 新设计**:
```csharp
public static unsafe class ApplicationHost
{
    public static bool Initialize()
    {
        // 1. Native API Bootstrap (加载 DLL)
        NativeApiBootstrap.Initialize();

        // 2. 发现项目根目录
        var projectRoot = DiscoverProjectRoot();

        // 3. VFS 初始化 (在 SDL 之前!)
        EditorVfsInitializer.InitializeVfs(projectRoot);

        // 4. SDL 初始化
        SdlApplicationHost.Initialize();

        // 5. 创建主窗口
        var mainWindow = SdlWindowManager.Create("Editor", 1280, 720, true, false, false);

        // 6. 创建 RenderSurface
        var renderSurface = RenderSurfaceHost.CreateSurface(mainWindow);

        // 7. ImGui Backend 初始化
        ImGuiBackendBridge.Initialize(
            SdlWindowManager.Resolve(mainWindow).SdlWindow,
            renderSurface.Device,
            renderSurface.Context,
            renderSurface.SwapChain);

        return true;
    }

    public static bool PumpEvents() => SdlEventBridge.PumpEvents();

    public static void BeginFrame()
    {
        ImGui.NewFrame();
        ImGuiBackendBridge.NewFrame();
    }

    public static void EndFrame()
    {
        ImGui.Render();
        ImGuiBackendBridge.Render();
    }

    public static void Shutdown()
    {
        ImGuiBackendBridge.Shutdown();
        RenderSurfaceHost.DestroyAll();
        SdlWindowManager.DestroyAll();
        SdlApplicationHost.Shutdown();
    }
}
```

**产出**: 完整的 C# 驱动启动链

---

### Phase 8: 清理 C++ 代码 (1天)

**目标**: 删除废弃的 C++ 代码，更新构建系统

**删除文件** (NNRuntimeApplication):
- `Private/BuildApplicationApi.cpp`
- `Private/BuildWindowApi.cpp`
- `Private/BuildEventApi.cpp`
- `Private/Core/Window.cpp`
- `Private/Core/WindowRegistry.cpp`
- `Private/Editor/EditorInitializer.cpp`
- `Private/Editor/EditorCore.cpp`
- `Private/Engine/ImGuiLayer.cpp` — 被 ImGui Backend 封装替代
- `Include/Core/EventQueue.h`
- `Include/Core/SDL3EventTranslator.h`
- `Include/Core/Window.h`
- `Include/Core/WindowRegistry.h`
- `Include/Editor/EditorInitializer.h`
- `Include/Editor/EditorCore.h`
- `Include/Engine/ImGuiLayer.h` — 被 ImGui Backend 头文件替代
- `Include/ApplicationApiExport.h`
- `Include/WindowApiExport.h`
- `Include/EventApiExport.h`

**新增文件**:
- `Private/ImGuiSDL3Backend.cpp`
- `Private/ImGuiDiligentBackend.cpp`
- `Include/ImGuiBackendApi.h`

**保留/修改文件**:
- `Private/RuntimeApplication.cpp` — 简化为仅 ImGui Backend 管理
- `Include/RuntimeApplication.h` — 简化
- `CMakeLists.txt` — 移除 SDL3 依赖，移除删除的文件，添加 ImGui Backend 文件

**ABI 变更**:
- `NNNativeEngineApi` 移除 `Application`, `Window`, `Events` 三个子表
- 新增 `ImGuiBackend` 子表 (5个函数)
- `LayoutVersion` 递增 (当前 32 → 33)

**产出**: C++ 模块精简为仅 ImGui Backend 封装

---

## 5. 文件变更汇总

### 5.1 C# 新增文件

| 文件路径 | 说明 |
|---------|------|
| `Neverness.Runtime.Application/Private/SdlApplicationHost.cs` | SDL3 生命周期 |
| `Neverness.Runtime.Application/Private/SdlWindowManager.cs` | 窗口注册表 |
| `Neverness.Runtime.Application/Private/SdlEventBridge.cs` | 事件泵 + 直接分发 |
| `Neverness.Runtime.Application/Private/RenderSurfaceHost.cs` | RenderSurface/SwapChain 管理 |
| `Neverness.Runtime.Application/Private/ImGuiBackendBridge.cs` | ImGui Backend C++/C# 桥接 |
| `Neverness.Runtime.Application/Private/EditorVfsInitializer.cs` | VFS 初始化 |
| `Neverness.Runtime.Application/Public/SdlWindow.cs` | 窗口 OOP 包装 |
| `Neverness.Runtime.Application/Public/WindowHandle.cs` | WindowHandle 结构 |
| `Neverness.Runtime.Application/Public/RenderSurface.cs` | RenderSurface 包装 |

### 5.2 C# 修改文件

| 文件路径 | 变更 |
|---------|------|
| `Neverness.Runtime.Application/Neverness.Runtime.Application.csproj` | 添加 SDL3-CS, ImGui.NET 包引用 |
| `Neverness.Runtime.Application/ApplicationHost.cs` | 重写 |
| `Neverness.Runtime.Application/Private/WindowHost.cs` | 删除 (被 SdlWindowManager 替代) |
| `Neverness.Runtime.Application/Public/Window.cs` | 删除 (被 SdlWindow 替代) |
| `Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | 移除旧子表，新增 ImGuiBackendApi |
| `Neverness.Runtime.Engine/EngineNativeApiBootstrap.cs` | 适配 LayoutVersion 变更 |
| `Neverness.Runtime.Bootstrap/RuntimeBootstrap.cs` | 适配新 ApplicationHost |

### 5.3 C++ 删除文件

| 文件路径 | 说明 |
|---------|------|
| `NNRuntimeApplication/Private/BuildApplicationApi.cpp` | ABI 导出不再需要 |
| `NNRuntimeApplication/Private/BuildWindowApi.cpp` | ABI 导出不再需要 |
| `NNRuntimeApplication/Private/BuildEventApi.cpp` | ABI 导出不再需要 |
| `NNRuntimeApplication/Private/Core/Window.cpp` | 窗口迁移到 C# |
| `NNRuntimeApplication/Private/Core/WindowRegistry.cpp` | 窗口注册表迁移到 C# |
| `NNRuntimeApplication/Private/Editor/EditorInitializer.cpp` | VFS 初始化迁移到 C# |
| `NNRuntimeApplication/Private/Editor/EditorCore.cpp` | Editor 路径实现 |
| `NNRuntimeApplication/Private/Engine/ImGuiLayer.cpp` | 被 ImGui Backend 封装替代 |
| `NNRuntimeApplication/Include/Core/EventQueue.h` | 事件队列不再需要 |
| `NNRuntimeApplication/Include/Core/SDL3EventTranslator.h` | 事件翻译不再需要 |
| `NNRuntimeApplication/Include/Core/Window.h` | 窗口头文件 |
| `NNRuntimeApplication/Include/Core/WindowRegistry.h` | 窗口注册表头文件 |
| `NNRuntimeApplication/Include/Editor/EditorInitializer.h` | VFS 初始化头文件 |
| `NNRuntimeApplication/Include/Editor/EditorCore.h` | Editor 路径常量 |
| `NNRuntimeApplication/Include/Engine/ImGuiLayer.h` | 被 ImGui Backend 头文件替代 |
| `NNRuntimeApplication/Include/ApplicationApiExport.h` | ABI 导出头文件 |
| `NNRuntimeApplication/Include/WindowApiExport.h` | ABI 导出头文件 |
| `NNRuntimeApplication/Include/EventApiExport.h` | ABI 导出头文件 |

### 5.4 C++ 新增文件

| 文件路径 | 说明 |
|---------|------|
| `NNRuntimeApplication/Private/ImGuiSDL3Backend.cpp` | ImGui SDL3 Backend 封装 |
| `NNRuntimeApplication/Private/ImGuiDiligentBackend.cpp` | ImGui Diligent Backend 封装 |
| `NNRuntimeApplication/Include/ImGuiBackendApi.h` | ImGui Backend 导出声明 |

### 5.5 C++ 修改文件

| 文件路径 | 变更 |
|---------|------|
| `NNRuntimeApplication/Include/RuntimeApplication.h` | 简化: 仅 ImGui Backend 管理 |
| `NNRuntimeApplication/Private/RuntimeApplication.cpp` | 简化: 仅 ImGui Backend 管理 |
| `NNRuntimeApplication/CMakeLists.txt` | 移除 SDL3 依赖，移除删除的文件，添加 ImGui Backend 文件 |
| `NNRuntimeEngineServices/Private/Diligent/DiligentRuntimeApi.cpp` | 适配新架构 |

---

## 6. 风险和待确认问题

### 6.1 高风险项

| # | 风险 | 影响 | 缓解措施 | 验证时机 |
|---|------|------|---------|---------|
| R1 | **DiligentEngine.NET 的 IDevice 是否为 COM 指针包装** | ImGui Backend 无法使用 C# 创建的设备 | **Phase 1 PoC 验证** | 最高优先级 |
| R2 | SDL3-CS 的 `SDL_Window*` 能否跨 C#/C++ 共享 | ImGui Backend 无法获取窗口信息 | Phase 0 验证 | 高优先级 |
| R3 | SDL3-CS 的 `SDL_Event` 内存布局是否与 C 完全一致 | 事件传递给 C++ 时数据损坏 | Phase 0 验证 | 高优先级 |
| R4 | ImGui.NET 与 C++ ImGui Backend 的版本兼容性 | ImGui 内部数据结构不匹配 | Phase 6 验证 | 中优先级 |
| R5 | SDL3-CS NuGet 包是否有 Windows x64 native 二进制 | 无法运行 | Phase 0 验证 | 高优先级 |

### 6.2 待确认问题

| # | 问题 | 需要确认 |
|---|------|---------|
| Q1 | SDL3-CS NuGet 包的确切名称和版本 | 查 NuGet.org |
| Q2 | ImGui.NET NuGet 包的确切名称和版本 | 查 NuGet.org |
| Q3 | DiligentEngine.NET 的 IDevice/IDeviceContext/ISwapChain 内部实现 | 检查源码，确认是 IntPtr NativePointer |
| Q4 | 现有 C# Editor 代码中哪些直接依赖 `NativeEventPump` 和 `NNEvent` | 搜索代码 |
| Q5 | `EditorInitializer::PakResource()` 是否已有 C# 等价实现 | 搜索 VFS 相关 C# 代码 |
| Q6 | ImGui.NET 的版本应与 C++ ImGui Backend 使用的 ImGui 版本一致 | 确认版本号 |

---

## 7. 估算

| Phase | 工作量 | 依赖 | 说明 |
|-------|--------|------|------|
| Phase 0: SDL3-CS 验证 | 1天 | 无 | 验证 SDL3-CS 可用性 |
| Phase 1: Diligent.NET + ImGui PoC | 2天 | Phase 0 | **最大风险，最先验证** |
| Phase 2: 窗口管理 | 2天 | Phase 0 | WindowHandle = SDL_WindowID |
| Phase 3: 事件系统 | 1天 | Phase 0 | 直接分发 SDL_Event |
| Phase 4: VFS 初始化 | 1天 | 无 | VFS 在 SDL 之前初始化 |
| Phase 5: RenderSurface 分离 | 1天 | Phase 2 | Window 和 SwapChain 分离 |
| Phase 6: ImGui Backend 封装 | 2天 | Phase 1, Phase 5 | C++ 极小封装 + ImGui.NET |
| Phase 7: 生命周期整合 | 1天 | Phase 2-6 | 完整 C# 启动链 |
| Phase 8: C++ 清理 | 1天 | Phase 7 | 删除废弃代码 |
| **总计** | **12天** | | |

---

## 8. 回滚策略

每个 Phase 独立可验证。如果某个 Phase 遇到不可行的技术障碍:
- Phase 0-1: 直接回滚，C# 模块恢复为调用 C++ Native API
- Phase 1 失败: Diligent 设备创建保留在 C++ 端，C# 只获取引用
- Phase 2-5: 可独立回滚，窗口/事件/VFS/RenderSurface 可混合 C++/C#
- Phase 6 失败: ImGui 保留在 C++ 端，但仍然精简为 Backend 封装

C++ `NNRuntimeApplication` 模块在 Phase 8 之前始终保持可编译状态，作为回滚后备。

---

## 9. 与 v1 的主要差异

| 项目 | v1 | v2 | 理由 |
|------|----|----|------|
| ImGui 方案 | C++ ImGuiLayer + NNImGuiApi | ImGui.NET + 极小 C++ Backend | 避免新增 ABI，职责清晰 |
| WindowHandle | 自增 ulong | SDL_WindowID (uint32_t) | 无需双重映射 |
| EventBridge | TranslateEvent → NNEvent | 直接分发 SDL_Event | 不需要兼容旧结构 |
| Diligent 创建 | 在 SdlWindow 中 | 独立 RenderSurfaceHost | 支持多窗口 |
| VFS 初始化顺序 | SDL Init → VFS | VFS → SDL Init | VFS 不依赖 SDL |
| Phase 顺序 | 0→1→2→3→4→5→6→7 | 0→1(风险前置)→2→3→4→5→6→7→8 | 最大风险最先验证 |
