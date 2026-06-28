# SdlEventBridge 窗口路由分发计划

> **创建日期**: 2026-06-28
> **状态**: 待审查
> **文件**: `Private/SdlEventBridge.cs`, `Public/SdlWindow.cs`, `Public/SdlWindowManager.cs`, **新建** `Public/SdlWindowEvents.cs`

---

## 1. 核心思路

```
SDL_PollEvent → SdlEventBridge.PumpEvents()
    │
    ├── OnEvent?.Invoke(e)                    ← 原始广播（ImGui 等）
    │
    ├── ExtractWindowId(e) → FindBySdlId → SdlWindow.Events.Dispatch(e)
    │
    └── QUIT → ShouldQuit
```

**职责分离**:
- `SdlWindow` — 窗口操作（创建/销毁/几何/状态），不关心事件
- `SdlWindowEvents` — 事件回调 + 分发，持有窗口的所有事件订阅
- `SdlEventBridge` — 泵送 + 路由，不持有事件逻辑

---

## 2. 改动

### 2.1 SdlWindowManager — FindBySdlId O(n) → O(1)

```csharp
public static WindowHandle FindBySdlId(uint sdlWindowId)
{
    var handle = new WindowHandle(sdlWindowId);
    return s_windows.ContainsKey(handle) ? handle : WindowHandle.Invalid;
}
```

### 2.2 新建 SdlWindowEvents.cs

```csharp
// Neverness.Runtime.Application — 窗口事件分发器。
// 持有单个 SdlWindow 的所有事件回调。
// 由 SdlEventBridge 路由调用，外部不应直接调用 Dispatch。

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 窗口事件分发器。
/// 每个 SdlWindow 持有一个实例，通过 SdlWindow.Events 访问。
/// </summary>
public sealed class SdlWindowEvents
{
    // ── 窗口事件 ──
    public event Action<int, int>? OnResized;
    public event Action<int, int>? OnPixelSizeChanged;
    public event Action? OnCloseRequested;
    public event Action<bool>? OnFocusChanged;
    public event Action<bool>? OnMinimizedChanged;
    public event Action<int, int>? OnMoved;
    public event Action<float>? OnDisplayScaleChanged;

    // ── 输入事件 ──
    public event Action<SDL.SDL_KeyboardEvent>? OnKeyDown;
    public event Action<SDL.SDL_KeyboardEvent>? OnKeyUp;
    public event Action<SDL.SDL_MouseButtonEvent>? OnMouseButtonDown;
    public event Action<SDL.SDL_MouseButtonEvent>? OnMouseButtonUp;
    public event Action<SDL.SDL_MouseMotionEvent>? OnMouseMotion;
    public event Action<SDL.SDL_MouseWheelEvent>? OnMouseWheel;
    public event Action<SDL.SDL_GamepadButtonEvent>? OnGamepadButtonDown;
    public event Action<SDL.SDL_GamepadButtonEvent>? OnGamepadButtonUp;
    public event Action<SDL.SDL_GamepadAxisEvent>? OnGamepadAxis;

    // ── 拖放/文本事件 ──
    public event Action<string>? OnDropFile;
    public event Action<string>? OnDropText;
    public event Action<string>? OnTextInput;

    /// <summary>统一事件分发入口。由 SdlEventBridge 调用。</summary>
    internal void Dispatch(SDL.SDL_Event e)
    {
        switch (e.Type)
        {
            // 窗口事件
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESIZED:
                OnResized?.Invoke(e.window.data1, e.window.data2); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                OnPixelSizeChanged?.Invoke(e.window.data1, e.window.data2); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                OnCloseRequested?.Invoke(); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_GAINED:
                OnFocusChanged?.Invoke(true); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_LOST:
                OnFocusChanged?.Invoke(false); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_MINIMIZED:
                OnMinimizedChanged?.Invoke(true); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESTORED:
                OnMinimizedChanged?.Invoke(false); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_MOVED:
                OnMoved?.Invoke(e.window.data1, e.window.data2); break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                OnDisplayScaleChanged?.Invoke(e.window.data1 / 1000f); break;

            // 键盘
            case SDL.SDL_EventType.SDL_EVENT_KEY_DOWN:
                OnKeyDown?.Invoke(e.key); break;
            case SDL.SDL_EventType.SDL_EVENT_KEY_UP:
                OnKeyUp?.Invoke(e.key); break;

            // 鼠标
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN:
                OnMouseButtonDown?.Invoke(e.button); break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP:
                OnMouseButtonUp?.Invoke(e.button); break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION:
                OnMouseMotion?.Invoke(e.motion); break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL:
                OnMouseWheel?.Invoke(e.wheel); break;

            // 手柄
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                OnGamepadButtonDown?.Invoke(e.gbutton); break;
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP:
                OnGamepadButtonUp?.Invoke(e.gbutton); break;
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION:
                OnGamepadAxis?.Invoke(e.gaxis); break;

            // 拖放
            case SDL.SDL_EventType.SDL_EVENT_DROP_FILE:
                OnDropFile?.Invoke(e.drop.GetData() ?? string.Empty); break;
            case SDL.SDL_EventType.SDL_EVENT_DROP_TEXT:
                OnDropText?.Invoke(e.drop.GetData() ?? string.Empty); break;

            // 文本输入
            case SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT:
                OnTextInput?.Invoke(e.text.GetText() ?? string.Empty); break;
        }
    }
}
```

### 2.3 SdlWindow — 添加 Events 属性

```csharp
public sealed unsafe class SdlWindow : IDisposable
{
    // ... 现有字段不变 ...

    /// <summary>窗口事件分发器。</summary>
    public SdlWindowEvents Events { get; } = new();

    // ... 现有方法不变 ...
}
```

### 2.4 SdlEventBridge — 纯路由

```csharp
internal static unsafe class SdlEventBridge
{
    private static bool s_shouldQuit;

    public static bool ShouldQuit => s_shouldQuit;

    /// <summary>原始事件广播（ImGui 等，路由之前调用）。</summary>
    public static event Action<SDL.SDL_Event>? OnEvent;

    public static bool PumpEvents()
    {
        SDL.SDL_Event e;
        while (SDL.SDL3.SDL_PollEvent(&e))
        {
            OnEvent?.Invoke(e);

            if (e.Type is SDL.SDL_EventType.SDL_EVENT_QUIT
                        or SDL.SDL_EventType.SDL_EVENT_TERMINATING)
            {
                s_shouldQuit = true;
                continue;
            }

            var windowId = ExtractWindowId(e);
            if (windowId != 0)
            {
                var window = SdlWindowManager.Resolve(new WindowHandle(windowId));
                window?.Events.Dispatch(e);
            }
        }
        return !s_shouldQuit;
    }

    public static void ResetQuitFlag() => s_shouldQuit = false;

    private static uint ExtractWindowId(SDL.SDL_Event e) => e.Type switch
    {
        >= SDL.SDL_EventType.SDL_EVENT_WINDOW_FIRST
        and <= SDL.SDL_EventType.SDL_EVENT_WINDOW_LAST
            => (uint)e.window.windowID,
        SDL.SDL_EventType.SDL_EVENT_KEY_DOWN
        or SDL.SDL_EventType.SDL_EVENT_KEY_UP
            => (uint)e.key.windowID,
        SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN
        or SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP
            => (uint)e.button.windowID,
        SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION
            => (uint)e.motion.windowID,
        SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL
            => (uint)e.wheel.windowID,
        SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN
        or SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP
        or SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION
            => GetFocusedWindowId(),
        SDL.SDL_EventType.SDL_EVENT_DROP_FILE
        or SDL.SDL_EventType.SDL_EVENT_DROP_TEXT
            => (uint)e.drop.windowID,
        SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT
            => (uint)e.text.windowID,
        _ => 0,
    };

    private static uint GetFocusedWindowId()
    {
        var w = SDL.SDL3.SDL_GetKeyboardFocus();
        return w != null ? (uint)SDL.SDL3.SDL_GetWindowID(w) : 0;
    }
}
```

---

## 3. 使用方式

```csharp
var handle = SdlWindowManager.Create("Editor", 1280, 720);
var window = SdlWindowManager.Resolve(handle);

// 窗口事件
window.Events.OnResized += (w, h) => ResizeSwapChain(window, w, h);
window.Events.OnCloseRequested += () => RequestClose(window);
window.Events.OnDropFile += path => ImportAsset(path);

// 输入事件
window.Events.OnKeyDown += key => HandleKeyDown(key);
window.Events.OnMouseMotion += motion => HandleMouse(motion);
```

---

## 4. 删除的内容

| 删除 | 替代 |
|------|------|
| `OnWindowSizeChanged(uint, int, int)` | `SdlWindow.Events.OnResized` |
| `OnWindowClose(uint)` | `SdlWindow.Events.OnCloseRequested` |
| `OnWindowFocus(uint, bool)` | `SdlWindow.Events.OnFocusChanged` |
| `OnWindowMinimized(uint, bool)` | `SdlWindow.Events.OnMinimizedChanged` |
| `OnDropFile(uint, string)` | `SdlWindow.Events.OnDropFile` |
| `OnDropText(uint, string)` | `SdlWindow.Events.OnDropText` |
| `OnTextInput(uint, string)` | `SdlWindow.Events.OnTextInput` |
| `Console.WriteLine(e.Type.ToString())` | 删除 |

---

## 5. 执行步骤

| Step | 文件 | 改动 |
|------|------|------|
| 1 | `SdlWindowManager.cs` | `FindBySdlId` 改为 O(1) |
| 2 | **新建** `SdlWindowEvents.cs` | 事件回调 + `Dispatch` |
| 3 | `SdlWindow.cs` | 添加 `Events` 属性 |
| 4 | `SdlEventBridge.cs` | 删除旧事件，只留 `OnEvent` + `ExtractWindowId` + 路由 |
| 5 | 消费方 | 适配 `window.Events.OnXxx` |
