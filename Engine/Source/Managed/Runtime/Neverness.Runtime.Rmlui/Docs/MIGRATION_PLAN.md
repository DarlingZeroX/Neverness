# NNRuntimeRmlui C# 迁移计划

> 从 C++ `NNRuntimeRmlui` 迁移到 C# `Neverness.Runtime.Rmlui`
> 创建日期：2026-06-27
> 更新日期：2026-06-27（Phase 2 完成）

---

## 1. 架构概述

### 1.1 分工

| 层 | 职责 | 实现方式 |
|---|------|----------|
| **C#** | 文档管理、输入处理、更新逻辑、同步 | RmlUi.Net |
| **C++** | GPU 渲染 | RmlRenderer（新版） |
| **连接** | C# 把 Handle 传给 ViewportSurface | ABI: Create/Destroy |

### 1.2 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    C# 逻辑层                                │
│  RmlUISystem、RmlRenderer、RmlDocument、RmlSdlPlatform      │
│  RmlShell、RmlHotReloader、RmlSyncManager                   │
└─────────────────────────┬───────────────────────────────────┘
                          │ Handle (uint32_t)
┌─────────────────────────▼───────────────────────────────────┐
│                    ViewportSurface                           │
│  接收 Handle，调用 C++ RmlRenderer 渲染                     │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────────┐
│                    C++ RmlRenderer（新版）                   │
│  - Rml::Context 管理                                        │
│  - RmlDiligentRenderInterface                               │
│  - 文档管理（通过 VFS 路径）                                 │
│  - 渲染                                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. C++ 新增文件

```
Include/Renderer/RmlRenderer.h          ← 新版渲染器（从 RmlUIRenderer 复制）
Include/ABI/RmlRendererAbi.h            ← ABI 接口定义
Source/Renderer/RmlRenderer.cpp         ← 新版渲染器实现
Source/ABI/RmlRendererAbi.cpp           ← ABI Handle 管理
```

---

## 3. C# 文件结构

```
Neverness.Runtime.Rmlui/
├── Docs/
│   └── MIGRATION_PLAN.md
├── Internal/
│   └── RmlNativeInterop.cs     ← LibraryImport (Create/Destroy)
├── Public/
│   ├── RmlRenderer.cs          ← Handle 封装 + RmlUi.Net
│   ├── RmlDocument.cs          ← 文档封装
│   ├── RmlUISystem.cs          ← 全局系统（静态门面）
│   ├── RmlSdlPlatform.cs       ← SDL 平台层
│   ├── RmlShell.cs             ← Shell 工具类
│   ├── RmlHotReloader.cs       ← 热重载管理器
│   └── RmlSyncManager.cs       ← 文档同步管理器
└── Neverness.Runtime.Rmlui.csproj
```

---

## 4. API 参考

### 4.1 RmlUISystem（静态门面）

```csharp
public static class RmlUISystem
{
    // ── 初始化/关闭 ──
    public static void Initialize(int width, int height);
    public static void Shutdown();
    public static bool IsInitialized { get; }

    // ── 渲染器 ──
    public static RmlRenderer? Renderer { get; }
    public static uint RendererHandle { get; }
    public static RmlHotReloader? HotReloader { get; }
    public static RmlSyncManager? SyncManager { get; }

    // ── 文档管理 ──
    public static RmlDocument? LoadDocument(string path, bool autoShow = true);
    public static RmlDocument? GetDocument(string path);
    public static void ReloadAllDocuments();

    // ── 输入处理 ──
    public static bool OnMouseMove(int x, int y, KeyModifier modifiers);
    public static bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers);
    public static bool OnMouseWheel(float delta, KeyModifier modifiers);
    public static bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f);
    public static bool OnTextInput(string text);
    public static void OnMouseLeave();

    // ── 更新 ──
    public static void Update(float deltaTime);

    // ── 字体 ──
    public static bool LoadFontFace(string path, bool fallback = false);
    public static void LoadDefaultFonts(string fontsPath);

    // ── 调试 ──
    public static void SetDebuggerVisible(bool visible);
    public static void ToggleDebugger();

    // ── 热重载 ──
    public static void NotifyFileChanged(string filePath);
    public static void RegisterDocumentForReload(RmlDocument document, IEnumerable<string>? watchedFiles = null);

    // ── 文档同步 ──
    public static void SyncDocuments(IEnumerable<string> paths);
    public static bool SyncAddDocument(string path);
    public static void SyncRemoveDocument(string path);
    public static void SyncClearDocuments();
    public static void SyncReloadAllDocuments();
}
```

### 4.2 RmlRenderer

```csharp
public sealed class RmlRenderer : IDisposable
{
    // ── 属性 ──
    public uint Handle { get; }
    public IReadOnlyList<RmlDocument> Documents { get; }
    public RmlHotReloader HotReloader { get; }
    public RmlSyncManager SyncManager { get; }
    public bool IsDebuggerVisible { get; }

    // ── 文档管理 ──
    public RmlDocument? LoadDocument(string path, bool autoShow = true);
    public RmlDocument? GetDocument(string path);
    public void UnloadDocument(string path);
    public void ReloadAllDocuments();

    // ── 输入处理（带快捷键支持） ──
    public bool OnMouseMove(int x, int y, KeyModifier modifiers);
    public bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers);
    public bool OnMouseWheel(float delta, KeyModifier modifiers);
    public bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f);
    public bool OnTextInput(string text);
    public void OnMouseLeave();

    // ── 更新 ──
    public void Update(float deltaTime);

    // ── 字体 ──
    public bool LoadFontFace(string path, bool fallback = false);
    public void LoadDefaultFonts(string fontsPath);

    // ── 调试 ──
    public void SetDebuggerVisible(bool visible);
    public void ToggleDebugger();
}
```

### 4.3 RmlDocument

```csharp
public sealed class RmlDocument : IDisposable
{
    // ── 属性 ──
    public string Path { get; }
    public bool IsValid { get; }
    public bool IsVisible { get; }
    public RmlRenderer Renderer { get; }

    // ── 文档操作 ──
    public void Show(ModalFlag modalFlag = ModalFlag.None, FocusFlag focusFlag = FocusFlag.Auto);
    public void Hide();
    public void Close();
    public void Reload();
    public void PullToFront();

    // ── 元素访问 ──
    public Element? GetElementById(string id);
    public Element? QuerySelector(string selector);

    // ── 属性 ──
    public string GetTitle();
    public void SetTitle(string title);
    public string GetSourceURL();

    // ── 样式表 ──
    public void AddStyleSheetContainer(StyleSheetContainer container);
    public void ReloadStyleSheet();

    // ── 事件 ──
    public void AddEventListener(string eventName, EventListener listener, bool inCapturePhase = false);
    public void RemoveEventListener(string eventName, EventListener listener, bool inCapturePhase = false);

    // ── 底层访问 ──
    public ElementDocument? GetDocument();
    public IntPtr NativePtr { get; }
}
```

### 4.4 RmlSyncManager

```csharp
public sealed class RmlSyncManager
{
    // ── 同步 ──
    public void Sync(IEnumerable<string> paths);   // 三路 Diff
    public bool Add(string path);                   // 添加单个
    public void Remove(string path);                // 移除单个
    public void Clear();                            // 清除所有
    public void ReloadAll();                        // 重载所有活跃文档

    // ── 属性 ──
    public IReadOnlyCollection<string> ActivePaths { get; }
}
```

### 4.5 RmlHotReloader

```csharp
public sealed class RmlHotReloader : IDisposable
{
    // ── 文件监听 ──
    public void RegisterDocument(RmlDocument document, IEnumerable<string>? watchedFiles = null);
    public void UnregisterDocument(RmlDocument document);
    public void NotifyFileChanged(string filePath);

    // ── 事件 ──
    public event Action<string>? OnFileChanged;
}
```

### 4.6 RmlSdlPlatform

```csharp
public sealed unsafe class RmlSdlPlatform : IDisposable
{
    // ── 构造 ──
    public RmlSdlPlatform(SDL.SDL_Window* window);

    // ── SystemInterface ──
    public double GetElapsedTime();
    public void SetMouseCursor(string cursorName);
    public void SetClipboardText(string text);
    public string GetClipboardText();
    public void ActivateKeyboard(float caretX, float caretY, float lineHeight);
    public void DeactivateKeyboard();

    // ── 事件处理 ──
    public bool HandleEvent(Context context, SDL.SDL_Event @event);

    // ── 静态工具 ──
    public static KeyModifier GetKeyModifierState();
    public static KeyIdentifier ConvertKey(SDL.SDL_Keycode sdlKey);
    public static int ConvertMouseButton(byte sdlButton);
}
```

### 4.7 RmlShell

```csharp
public static class RmlShell
{
    // ── 字体加载 ──
    public static void LoadFonts(string fontsPath);
    public static void LoadDefaultFonts(string engineResourcePath);

    // ── 键盘快捷键 ──
    public static bool ProcessKeyDownShortcuts(Context context, KeyIdentifier key, KeyModifier keyModifier, float nativeDpRatio, bool priority);
    public static bool HandleDebugShortcut(KeyIdentifier key);
}
```

---

## 5. 使用示例

### 5.1 基本初始化和渲染

```csharp
// 初始化
RmlUISystem.Initialize(1920, 1080);

// 加载字体
RmlUISystem.LoadDefaultFonts("/assets/fonts");

// 加载文档
var doc = RmlUISystem.LoadDocument("/ui/main.rml");
doc?.SetTitle("My UI");

// 游戏循环
while (running)
{
    // 处理输入
    RmlUISystem.OnMouseMove(mouseX, mouseY, modifiers);
    RmlUISystem.OnKey(key, true, modifiers);

    // 更新
    RmlUISystem.Update(deltaTime);

    // 渲染（传给 ViewportSurface）
    ViewportSurface_RenderRml(surface, RmlUISystem.RendererHandle);
}

// 清理
RmlUISystem.Shutdown();
```

### 5.2 文档同步（三路 Diff）

```csharp
// 初始加载
RmlUISystem.SyncDocuments(["/ui/main.rml", "/ui/hud.rml"]);

// 帧更新：同步 DrawList
var activeDocuments = scene.GetActiveUIDocuments();
RmlUISystem.SyncDocuments(activeDocuments);

// 添加单个文档
RmlUISystem.SyncAddDocument("/ui/dialog.rml");

// 移除单个文档
RmlUISystem.SyncRemoveDocument("/ui/dialog.rml");

// 清除所有文档
RmlUISystem.SyncClearDocuments();

// 重新加载所有活跃文档
RmlUISystem.SyncReloadAllDocuments();
```

### 5.3 热重载

```csharp
// 文件变化时通知
RmlUISystem.NotifyFileChanged("/ui/main.rml");

// 或者使用 HotReloader
var reloader = RmlUISystem.HotReloader;

// 注册文档监听（监听 .rml 和 .rcss 文件）
reloader.RegisterDocument(doc, ["/ui/main.rml", "/ui/main.rcss"]);

// 文件变化时自动重载关联文档
reloader.NotifyFileChanged("/ui/main.rcss");

// 监听文件变化事件
reloader.OnFileChanged += (path) => Console.WriteLine($"File changed: {path}");
```

### 5.4 SDL 事件处理

```csharp
// 创建 SDL 平台层
var platform = new RmlSdlPlatform(sdlWindow);

// 处理 SDL 事件
SDL.SDL_Event e;
while (SDL.SDL3.SDL_PollEvent(&e))
{
    // 处理 RmlUI 输入
    platform.HandleEvent(context, e);

    // 处理其他事件
    switch (e.Type)
    {
        case SDL.SDL_EventType.SDL_EVENT_QUIT:
            running = false;
            break;
    }
}
```

### 5.5 键盘快捷键

```csharp
// 在输入处理中集成快捷键
bool OnKeyDown(KeyIdentifier key, KeyModifier modifiers)
{
    // 优先级快捷键（F8 切换调试器等）
    bool propagate = RmlShell.ProcessKeyDownShortcuts(
        context, key, modifiers, nativeDpRatio, priority: true);

    if (!propagate) return true;

    // 提交给 Context 处理
    bool handled = context.ProcessKeyDown(key, modifiers);

    // 低优先级快捷键（Ctrl+R 重载样式表等）
    if (!handled)
    {
        propagate = RmlShell.ProcessKeyDownShortcuts(
            context, key, modifiers, nativeDpRatio, priority: false);
    }

    return handled || !propagate;
}
```

### 5.6 文档操作

```csharp
// 加载文档
var doc = RmlUISystem.LoadDocument("/ui/main.rml");

// 显示/隐藏
doc.Show();
doc.Hide();

// 获取元素
var button = doc.GetElementById("myButton");
var element = doc.QuerySelector(".my-class");

// 设置属性
doc.SetTitle("My Document");

// 重新加载
doc.Reload();

// 监听事件
var listener = new EventListener((evt) => {
    Console.WriteLine($"Event: {evt.GetId()}");
});
doc.AddEventListener("click", listener);

// 销毁
doc.Dispose();
```

---

## 6. 实施状态

### Phase 1 ✅ 完成（C# 基础架构）

| 文件 | 状态 | 说明 |
|------|------|------|
| `Internal/RmlNativeInterop.cs` | ✅ | LibraryImport (Create/Destroy) |
| `Public/RmlRenderer.cs` | ✅ | Handle 封装 + RmlUi.Net |
| `Public/RmlDocument.cs` | ✅ | 文档封装 |
| `Public/RmlUISystem.cs` | ✅ | 全局系统 |

### Phase 2 ✅ 完成（逻辑完善）

| 文件 | 状态 | 说明 |
|------|------|------|
| `Public/RmlSdlPlatform.cs` | ✅ | SDL 平台层 |
| `Public/RmlShell.cs` | ✅ | Shell 工具类 |
| `Public/RmlHotReloader.cs` | ✅ | 热重载管理器 |
| `Public/RmlSyncManager.cs` | ✅ | 文档同步管理器 |

### Phase 3 🔄 进行中（C++ ABI）

| 文件 | 状态 | 说明 |
|------|------|------|
| `Include/Renderer/RmlRenderer.h` | ✅ | 新版渲染器头文件 |
| `Source/Renderer/RmlRenderer.cpp` | ✅ | 新版渲染器实现 |
| `Include/ABI/RmlRendererAbi.h` | ✅ | ABI 接口定义 |
| `Source/ABI/RmlRendererAbi.cpp` | ✅ | ABI Handle 管理 |

待完成：
- [ ] 集成渲染设备获取
- [ ] ViewportSurface 集成
- [ ] 编译验证

### Phase 4 待开始（系统集成）

- [ ] ECS 组件集成
- [ ] 编辑器集成
- [ ] 功能测试

---

## 7. C++ 渲染器对比

| 特性 | RmlUIRenderer（Legacy） | RmlRenderer（新版） |
|------|------------------------|---------------------|
| **依赖** | NNRuntimeScene | 无 |
| **文档标识** | NNGuid / NNEntity | VFS 路径 |
| **文档管理** | Sync/DrawList | 直接路径 |
| **用途** | 旧系统兼容 | C# ABI 调用 |

---

## 8. 待完成

1. **C++ 渲染设备获取**
   - `RmlRendererAbi.cpp` 中的 `GetRenderDevice()` 需要从引擎获取设备

2. **ViewportSurface 集成**
   - 把 Handle 传给 ViewportSurface 执行渲染

3. **RmlUi.Net API 扩展**
   - `SetDensityIndependentPixelRatio`
   - `GetNumDocuments` / `GetDocument`
   - `ReloadStyleSheet`

---

## 9. 参考资料

- [RmlRenderer 源码](../../../../Runtime/NNRuntimeRmlui/Source/Renderer/RmlRenderer.cpp)
- [RmlRendererAbi 源码](../../../../Runtime/NNRuntimeRmlui/Source/ABI/RmlRendererAbi.cpp)
- [RmlUi.Net 文档](../../ThirdParty/RmlUi.Net/README.md)
- [RmlDiligent 设计文档](../../../../Rendering/Docs/Plans/RmlDiligent-Plan.md)
