# SDL 输入层集成计划

## 1. 架构设计

### 1.1 当前架构

```
┌─────────────────────────────────────────────────────────────┐
│                    AvaloniaFrontend                         │
│  ┌───────────────────────────────────────────────────────┐  │
│  │           ViewportAvaloniaView                        │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │     ViewportHostService                         │  │  │
│  │  │  ┌─────────────────────────────────────────┐    │  │  │
│  │  │  │   NativeControlHostSurface              │    │  │  │
│  │  │  │  ┌─────────────────────────────────┐    │    │  │  │
│  │  │  │  │   ViewportHostControl           │    │    │  │  │
│  │  │  │  │   (Avalonia NativeControlHost)  │    │    │  │  │
│  │  │  │  └─────────────────────────────────┘    │    │  │  │
│  │  │  └─────────────────────────────────────────┘    │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
                    DumbWindow (HWND)
                            │
                            ▼
                    Diligent SwapChain
```

### 1.2 目标架构

```
┌─────────────────────────────────────────────────────────────┐
│                    AvaloniaFrontend                         │
│  ┌───────────────────────────────────────────────────────┐  │
│  │           ViewportAvaloniaView                        │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │     ViewportHostService                         │  │  │
│  │  │  ┌─────────────────────────────────────────┐    │  │  │
│  │  │  │   NativeControlHostSurface              │    │  │  │
│  │  │  │  ┌─────────────────────────────────┐    │    │  │  │
│  │  │  │  │   ViewportHostControl           │    │    │  │  │
│  │  │  │  │   (Avalonia NativeControlHost)  │    │    │  │  │
│  │  │  │  └─────────────────────────────────┘    │    │  │  │
│  │  │  └─────────────────────────────────────────┘    │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
                    DumbWindow (HWND)
                            │
              ┌─────────────┴─────────────┐
              ▼                           ▼
    SDL Input Window              Diligent SwapChain
    (SDL_WINDOW_EXTERNAL)         (渲染)
              │
              ▼
        SdlEventBridge
        (键盘/鼠标/游戏手柄)
```

### 1.3 核心设计

**职责分离**：
- **Avalonia**：窗口管理、UI 布局、窗口事件（resize/close/focus）
- **SDL**：输入处理（键盘、鼠标、游戏手柄）
- **Diligent**：渲染

**SDL 输入窗口**：
- 使用 `SDL_CreateWindowWithProperties` 创建
- 设置 `SDL_WINDOW_EXTERNAL` 标志（不创建原生窗口）
- 设置 `SDL_PROP_WINDOW_CREATE_PARENT_POINTER` 为 DumbWindow 的 HWND
- SDL 只接管输入，不管理窗口生命周期

**多窗口支持**：
- 每个视口窗口对应一个 `SdlInputWindow` 实例
- 通过 `SdlInputWindowManager` 统一管理所有实例
- 支持多个视口同时存在

**事件流**：
```
用户输入 → OS → SDL Input Window → SdlEventBridge → Editor/Input
窗口事件 → OS → Avalonia → ViewportAvaloniaView → Resize/Close
```

---

## 2. 实现步骤

### Phase 1: SDL 输入窗口管理器 ✅ 已完成

**目标**：创建 SDL 输入窗口，绑定到 Avalonia 的 DumbWindow

**文件**：
- `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Public/SdlWindow.cs` (修改)
- `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Public/SdlWindowManager.cs` (修改)
- `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/ApplicationHost.cs` (修改)

**实现内容**：

#### SdlWindow 扩展（支持输入窗口模式）
```csharp
/// <summary>
/// SDL3 窗口对象封装。
/// 支持两种模式：
/// 1. 普通窗口：完整的 SDL 窗口，支持渲染、输入等
/// 2. 输入窗口：绑定到 Avalonia DumbWindow，只接管输入
/// </summary>
public sealed unsafe class SdlWindow : IDisposable
{
    // 新增属性
    /// <summary>是否为输入窗口（绑定到外部 HWND，只接管输入）。</summary>
    public bool IsInputOnly { get; }

    /// <summary>父窗口 HWND（仅输入窗口有效）。</summary>
    public IntPtr ParentHwnd { get; }

    // 构造函数
    /// <summary>创建普通 SDL 窗口。</summary>
    internal SdlWindow(SDL.SDL_Window* window, WindowHandle handle, bool isPrimary = false);

    /// <summary>创建输入窗口（绑定到外部 HWND）。</summary>
    internal SdlWindow(SDL.SDL_Window* window, WindowHandle handle, IntPtr parentHwnd);
}
```

#### SdlWindowManager 统一管理（支持创建输入窗口）
```csharp
/// <summary>
/// SDL3 窗口注册表。
/// 统一管理普通窗口和输入窗口。
/// </summary>
public static unsafe class SdlWindowManager
{
    /// <summary>
    /// 创建 SDL 输入窗口（绑定到外部 HWND，只接管输入）。
    ///
    /// 工作原理：
    /// 1. 接收 Avalonia 的 HWND
    /// 2. 使用 SDL_CreateWindowWithProperties 创建 SDL 窗口
    /// 3. 设置 SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER 为 HWND
    /// 4. 设置 hidden=true 和 focusable=false
    /// 5. SDL 事件泵接收输入事件
    /// </summary>
    /// <param name="parentHwnd">Avalonia DumbWindow 的 HWND</param>
    /// <param name="width">初始宽度（像素）</param>
    /// <param name="height">初始高度（像素）</param>
    /// <returns>窗口句柄，失败返回 WindowHandle.Invalid</returns>
    public static WindowHandle CreateInputWindow(IntPtr parentHwnd, int width, int height);

    /// <summary>更新输入窗口尺寸。</summary>
    public static void UpdateInputWindowSize(WindowHandle handle, int width, int height);

    /// <summary>获取所有输入窗口。</summary>
    public static IEnumerable<SdlWindow> GetInputWindows();

    /// <summary>获取所有普通窗口。</summary>
    public static IEnumerable<SdlWindow> GetNormalWindows();
}
```

**关键实现细节**：
- 使用 `SDL_CreateWindowWithProperties` 创建窗口
- **设置 `SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER` 为 Avalonia DumbWindow 的 HWND**（SDL3 替代 SDL_CreateWindowFrom 的方式）
- 设置 `hidden=true` 和 `focusable=false`，SDL 只接管输入
- 使用 `WindowHandle`（基于 SDL_WindowID）作为句柄
- `IsInputOnly` 标志区分普通窗口和输入窗口

#### ApplicationHost 集成
```csharp
// 修改 Shutdown 方法
public static void Shutdown()
{
    // ... 其他代码 ...

    // 销毁所有窗口（包括普通窗口和输入窗口）
    SdlWindowManager.DestroyAll();

    // ... 其他代码 ...
}
```

**编译结果**：0 错误，1 警告（其他项目的警告）

**重构变更**：
- 删除了 `SdlInputWindow.cs` 和 `SdlInputWindowManager`
- 统一到 `SdlWindowManager` 管理所有 SDL 窗口
- `SdlWindow` 通过 `IsInputOnly` 标志区分窗口类型

---

### Phase 2: 修改 NativeControlHostSurface

**目标**：在原生句柄创建时，初始化 SDL 输入窗口

**文件**：
- `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Viewport/NativeControlHostSurface.cs` (修改)

**修改内容**：
```csharp
// 添加字段
private WindowHandle _sdlInputHandle;
private bool _sdlInputInitialized;

// 修改 OnHandleCreated 方法
private void OnHandleCreated(Avalonia.Platform.IPlatformHandle handle)
{
    _nativeHandle = handle.Handle;
    _handleDescriptor = handle.HandleDescriptor;
    _isValid = true;
    Console.WriteLine($"[NativeControlHostSurface] 原生句柄已获取: 0x{handle.Handle:X} ({handle.HandleDescriptor})");

    // 初始化 SDL 输入窗口
    if (handle.HandleDescriptor == "HWND")
    {
        _sdlInputHandle = SdlWindowManager.CreateInputWindow(handle.Handle, _width, _height);
        _sdlInputInitialized = _sdlInputHandle.IsValid;

        if (_sdlInputInitialized)
        {
            Console.WriteLine($"[NativeControlHostSurface] SDL 输入窗口初始化成功: {_sdlInputHandle}");
        }
    }

    SurfaceCreated?.Invoke(handle.Handle);
}

// 修改 Resize 方法
public void Resize(int width, int height)
{
    if (_width == width && _height == height)
        return;

    _width = width;
    _height = height;

    // 更新 SDL 输入窗口尺寸
    if (_sdlInputInitialized)
    {
        SdlWindowManager.UpdateInputWindowSize(_sdlInputHandle, width, height);
    }

    SurfaceResized?.Invoke(width, height);
}

// 修改 Dispose 方法
public void Dispose()
{
    if (_disposed)
        return;

    _disposed = true;
    _isValid = false;

    // 关闭 SDL 输入窗口
    if (_sdlInputInitialized)
    {
        SdlWindowManager.Destroy(_sdlInputHandle);
        _sdlInputHandle = WindowHandle.Invalid;
        _sdlInputInitialized = false;
    }

    if (_host != null)
    {
        _host.HandleCreated -= OnHandleCreated;
        _host.HandleDestroyed -= OnHandleDestroyed;
        _host = null;
    }

    _nativeHandle = IntPtr.Zero;
    SurfaceDestroyed?.Invoke();
}
```

**关键变更**：
- 使用 `SdlWindowManager.CreateInputWindow()` 创建输入窗口
- 使用 `SdlWindowManager.UpdateInputWindowSize()` 更新尺寸
- 使用 `SdlWindowManager.Destroy()` 销毁窗口
- 不再直接操作 `SdlInputWindow` 实例

---

### Phase 3: 修改 SdlEventBridge

**目标**：添加输入事件分发，区分窗口事件和输入事件

**文件**：
- `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Private/SdlEventBridge.cs` (修改)

**修改内容**：
```csharp
// 添加输入事件回调
/// <summary>键盘按下事件 (windowId, keyCode, scanCode, isRepeat)。</summary>
public static event Action<uint, SDL.SDL_Keycode, SDL.SDL_Scancode, bool>? OnKeyDown;

/// <summary>键盘释放事件 (windowId, keyCode, scanCode)。</summary>
public static event Action<uint, SDL.SDL_Keycode, SDL.SDL_Scancode>? OnKeyUp;

/// <summary>鼠标移动事件 (windowId, x, y, deltaX, deltaY)。</summary>
public static event Action<uint, float, float, float, float>? OnMouseMove;

/// <summary>鼠标按下事件 (windowId, button, x, y, clicks)。</summary>
public static event Action<uint, SDL.SDL_MouseButtonFlags, float, float, byte>? OnMouseDown;

/// <summary>鼠标释放事件 (windowId, button, x, y, clicks)。</summary>
public static event Action<uint, SDL.SDL_MouseButtonFlags, float, float, byte>? OnMouseUp;

/// <summary>鼠标滚轮事件 (windowId, x, y, direction)。</summary>
public static event Action<uint, float, float, SDL.SDL_MouseWheelDirection>? OnMouseWheel;

/// <summary>游戏手柄按钮按下事件 (joystickId, button)。</summary>
public static event Action<SDL.SDL_JoystickID, SDL.SDL_GamepadButton>? OnGamepadButtonDown;

/// <summary>游戏手柄按钮释放事件 (joystickId, button)。</summary>
public static event Action<SDL.SDL_JoystickID, SDL.SDL_GamepadButton>? OnGamepadButtonUp;

/// <summary>游戏手柄轴移动事件 (joystickId, axis, value)。</summary>
public static event Action<SDL.SDL_JoystickID, SDL.SDL_GamepadAxis, short>? OnGamepadAxis;

// 修改 PumpEvents 方法，添加输入事件分发
private static void DispatchInputEvent(SDL.SDL_Event e)
{
    switch (e.Type)
    {
        // 键盘事件
        case SDL.SDL_EventType.SDL_EVENT_KEY_DOWN:
            OnKeyDown?.Invoke(
                (uint)e.key.windowID,
                e.key.key,
                e.key.scancode,
                e.key.repeat);
            break;

        case SDL.SDL_EventType.SDL_EVENT_KEY_UP:
            OnKeyUp?.Invoke(
                (uint)e.key.windowID,
                e.key.key,
                e.key.scancode);
            break;

        // 鼠标事件
        case SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION:
            OnMouseMove?.Invoke(
                (uint)e.motion.windowID,
                e.motion.x,
                e.motion.y,
                e.motion.xrel,
                e.motion.yrel);
            break;

        case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN:
            OnMouseDown?.Invoke(
                (uint)e.button.windowID,
                e.button.button,
                e.button.x,
                e.button.y,
                e.button.clicks);
            break;

        case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP:
            OnMouseUp?.Invoke(
                (uint)e.button.windowID,
                e.button.button,
                e.button.x,
                e.button.y,
                e.button.clicks);
            break;

        case SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL:
            OnMouseWheel?.Invoke(
                (uint)e.wheel.windowID,
                e.wheel.x,
                e.wheel.y,
                e.wheel.direction);
            break;

        // 游戏手柄事件
        case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            OnGamepadButtonDown?.Invoke(
                e.gbutton.which,
                e.gbutton.button);
            break;

        case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP:
            OnGamepadButtonUp?.Invoke(
                e.gbutton.which,
                e.gbutton.button);
            break;

        case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION:
            OnGamepadAxis?.Invoke(
                e.gaxis.which,
                e.gaxis.axis,
                e.gaxis.value);
            break;
    }
}
```

---

### Phase 4: 修改 ViewportAvaloniaView

**目标**：订阅 SDL 输入事件，处理视口输入

**文件**：
- `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ViewportAvaloniaView.cs` (修改)

**修改内容**：
```csharp
// 添加字段
private bool _sdlInputSubscribed;

// 添加方法
private void SubscribeSdlInput()
{
    if (_sdlInputSubscribed) return;

    SdlEventBridge.OnKeyDown += OnSdlKeyDown;
    SdlEventBridge.OnKeyUp += OnSdlKeyUp;
    SdlEventBridge.OnMouseMove += OnSdlMouseMove;
    SdlEventBridge.OnMouseDown += OnSdlMouseDown;
    SdlEventBridge.OnMouseUp += OnSdlMouseUp;
    SdlEventBridge.OnMouseWheel += OnSdlMouseWheel;

    _sdlInputSubscribed = true;
}

private void UnsubscribeSdlInput()
{
    if (!_sdlInputSubscribed) return;

    SdlEventBridge.OnKeyDown -= OnSdlKeyDown;
    SdlEventBridge.OnKeyUp -= OnSdlKeyUp;
    SdlEventBridge.OnMouseMove -= OnSdlMouseMove;
    SdlEventBridge.OnMouseDown -= OnSdlMouseDown;
    SdlEventBridge.OnMouseUp -= OnSdlMouseUp;
    SdlEventBridge.OnMouseWheel -= OnSdlMouseWheel;

    _sdlInputSubscribed = false;
}

// 输入事件处理
private void OnSdlKeyDown(uint windowId, SDL.SDL_Keycode keyCode, SDL.SDL_Scancode scanCode, bool isRepeat)
{
    // TODO: 转发给 ViewportController 处理
    Console.WriteLine($"[Viewport] KeyDown: {keyCode} (repeat={isRepeat})");
}

private void OnSdlKeyUp(uint windowId, SDL.SDL_Keycode keyCode, SDL.SDL_Scancode scanCode)
{
    // TODO: 转发给 ViewportController 处理
}

private void OnSdlMouseMove(uint windowId, float x, float y, float deltaX, float deltaY)
{
    // TODO: 转发给 ViewportController 处理
}

private void OnSdlMouseDown(uint windowId, SDL.SDL_MouseButtonFlags button, float x, float y, byte clicks)
{
    // TODO: 转发给 ViewportController 处理
}

private void OnSdlMouseUp(uint windowId, SDL.SDL_MouseButtonFlags button, float x, float y, byte clicks)
{
    // TODO: 转发给 ViewportController 处理
}

private void OnSdlMouseWheel(uint windowId, float x, float y, SDL.SDL_MouseWheelDirection direction)
{
    // TODO: 转发给 ViewportController 处理
}

// 修改 Bind 方法
public override void Bind(object viewModel)
{
    // ... 现有代码 ...

    // 订阅 SDL 输入事件
    SubscribeSdlInput();

    // ... 现有代码 ...
}

// 修改 Unbind 方法
public override void Unbind()
{
    // 取消订阅 SDL 输入事件
    UnsubscribeSdlInput();

    // ... 现有代码 ...
}
```

---

### Phase 5: 双向 Resize 同步 + Debounce

**目标**：Avalonia Bounds 变化 → SDL 窗口同步尺寸，Debounce 后触发 SwapChain Resize

**文件**：
- `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Viewport/NativeControlHostSurface.cs` (修改)

**修改内容**：
```csharp
// 添加 Debounce 相关字段
private DateTime _lastResizeTime;
private readonly TimeSpan _resizeDebounce = TimeSpan.FromMilliseconds(16); // ~60fps
private Timer? _resizeTimer;
private int _pendingWidth;
private int _pendingHeight;

// 修改 Resize 方法
public void Resize(int width, int height)
{
    if (_width == width && _height == height)
        return;

    _width = width;
    _height = height;

    // 立即更新 SDL 输入窗口尺寸
    if (_sdlInputInitialized && _sdlInputWindow != null)
    {
        _sdlInputWindow.UpdateSize(width, height);
    }

    // Debounce 后触发 SurfaceResized（用于 SwapChain Resize）
    _pendingWidth = width;
    _pendingHeight = height;
    _lastResizeTime = DateTime.Now;

    _resizeTimer?.Dispose();
    _resizeTimer = new Timer(_ =>
    {
        var elapsed = DateTime.Now - _lastResizeTime;
        if (elapsed >= _resizeDebounce)
        {
            SurfaceResized?.Invoke(_pendingWidth, _pendingHeight);
        }
    }, null, _resizeDebounce, Timeout.InfiniteTimeSpan);
}

// 修改 Dispose 方法，清理 Timer
public void Dispose()
{
    if (_disposed)
        return;

    _disposed = true;
    _isValid = false;

    _resizeTimer?.Dispose();
    _resizeTimer = null;

    // ... 现有代码 ...
}
```

---

### Phase 6: ApplicationHost 集成 ✅ 已完成

**目标**：在 ApplicationHost 中集成 SDL 输入窗口生命周期

**文件**：
- `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/ApplicationHost.cs` (修改)

**实现内容**：
```csharp
// 修改 Shutdown 方法
public static void Shutdown()
{
    if (!s_initialized)
    {
        return;
    }

    // 关闭 ImGui Backend
    ImGuiBackendBridge.Shutdown();

    // 销毁所有渲染表面
    RenderSurfaceHost.DestroyAll();

    // 销毁所有窗口（包括普通窗口和输入窗口）
    SdlWindowManager.DestroyAll();

    // 关闭 SDL3
    SdlApplicationHost.Shutdown();

    s_initialized = false;
    Console.WriteLine("[ApplicationHost] 已关闭");
}
```

**编译结果**：0 错误

**关键变更**：
- 删除了 `SdlInputWindowManager.ShutdownAll()` 调用
- `SdlWindowManager.DestroyAll()` 统一销毁所有窗口（普通窗口 + 输入窗口）

---

## 3. 测试验证

### 3.1 单元测试

**文件**：
- `Engine/Source/Managed/Runtime/Tests/SdlInputWindowTests.cs` (新建)

**测试用例**：
1. `Initialize_WithValidHwnd_ReturnsTrue`
2. `Initialize_WithInvalidHwnd_ReturnsFalse`
3. `UpdateSize_WhenInitialized_DoesNotThrow`
4. `Dispose_WhenInitialized_ClearsState`
5. `Initialize_Twice_ReturnsTrueSecondTime`
6. `Manager_Register_IncreasesCount`
7. `Manager_Unregister_DecreasesCount`
8. `Manager_FindBySdlId_ReturnsCorrectWindow`

### 3.2 集成测试

**测试场景**：
1. 启动编辑器，创建视口窗口
2. 验证 SDL 输入窗口已创建
3. 在视口中按下键盘，验证 SdlEventBridge 收到事件
4. 在视口中移动鼠标，验证 SdlEventBridge 收到事件
5. 调整视口大小，验证 Debounce 后触发 SwapChain Resize
6. 创建多个视口窗口，验证多窗口支持

---

## 4. 风险和注意事项

### 4.1 风险

1. **SDL_CreateWindowWithProperties 可能不支持 parent**
   - 验证：检查 SDL3 文档和 SDL3-CS 绑定
   - 备选方案：使用平台特定 API（Windows: SetParent）

2. **SDL 输入窗口可能与 Avalonia 事件冲突**
   - 验证：测试键盘/鼠标事件是否被 SDL 拦截
   - 备选方案：只在视口聚焦时启用 SDL 输入

3. **跨平台兼容性**
   - Windows：HWND + SetParent
   - Linux：X11 Window + XReparentWindow
   - macOS：NSView + addSubview

### 4.2 注意事项

1. **线程安全**：SDL 事件泵在主线程调用，Avalonia 也在主线程，需要确保无死锁
2. **生命周期**：SDL 输入窗口必须在 Avalonia DumbWindow 销毁前关闭
3. **性能**：Debounce 间隔（16ms）需要根据实际帧率调整

---

## 5. 时间估算

| Phase | 任务 | 估算时间 | 实际状态 |
|-------|------|----------|----------|
| 1 | SDL 输入窗口管理器 | 2 小时 | ✅ 已完成 |
| 2 | 修改 NativeControlHostSurface | 1 小时 | 待实施 |
| 3 | 修改 SdlEventBridge | 1.5 小时 | 待实施 |
| 4 | 修改 ViewportAvaloniaView | 1 小时 | 待实施 |
| 5 | 双向 Resize 同步 + Debounce | 1.5 小时 | 待实施 |
| 6 | ApplicationHost 集成 | 0.5 小时 | ✅ 已完成 |
| 测试 | 单元测试 + 集成测试 | 2 小时 | 待实施 |
| **总计** | | **9.5 小时** | **2/6 完成** |

---

## 6. 依赖项

1. **SDL3-CS 2026.623.0**：需要验证 SDL_CreateWindowWithProperties 支持
2. **Avalonia NativeControlHost**：需要验证 DumbWindow 的 HWND 可以被 SDL 接管
3. **Diligent SwapChain**：需要验证 Resize 流程无冲突

---

## 7. 待确认问题

1. [x] SDL3-CS 是否有 `SDL_PROP_WINDOW_CREATE_PARENT_POINTER` 绑定？ ✅ 已验证：存在
2. [x] SDL_CreateWindowWithProperties 是否支持 `SDL_WINDOW_EXTERNAL` 标志？ ✅ 已验证：存在（值为 2048）
3. [x] SDL_CreateProperties API 是否可用？ ✅ 已验证：存在
4. [x] 多窗口支持？ ✅ 已重构为实例类 + 管理器
5. [ ] Avalonia DumbWindow 的 HWND 是否可以被 SDL 接管？（需要实际测试）
6. [ ] 输入事件是否需要过滤（只处理视口聚焦时的事件）？
7. [ ] Debounce 间隔（16ms）是否合适？

---

## 8. 代码变更摘要

### 已完成变更

#### 修改文件
1. `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Public/SdlWindow.cs`
   - 添加 `IsInputOnly` 属性（是否为输入窗口）
   - 添加 `ParentHwnd` 属性（父窗口 HWND，仅输入窗口有效）
   - 添加输入窗口构造函数 `SdlWindow(SDL_Window*, WindowHandle, IntPtr parentHwnd)`

2. `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Public/SdlWindowManager.cs`
   - 添加 `CreateInputWindow(IntPtr parentHwnd, int width, int height)` 方法
   - 使用 `SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER` 绑定 Avalonia HWND
   - 添加 `UpdateInputWindowSize(WindowHandle, int, int)` 方法
   - 添加 `GetInputWindows()` 和 `GetNormalWindows()` 查询方法

3. `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/ApplicationHost.cs`
   - 删除 `SdlInputWindowManager.ShutdownAll()` 调用
   - `SdlWindowManager.DestroyAll()` 统一销毁所有窗口

#### 删除文件
1. `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Private/SdlInputWindow.cs`
   - 功能已合并到 `SdlWindow` 和 `SdlWindowManager`

### 待修改文件
1. `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Viewport/NativeControlHostSurface.cs`
   - 使用 `SdlWindowManager.CreateInputWindow()` 创建输入窗口
   - 使用 `SdlWindowManager.UpdateInputWindowSize()` 更新尺寸
   - 使用 `SdlWindowManager.Destroy()` 销毁窗口

2. `Engine/Source/Managed/Runtime/Neverness.Runtime.Application/Private/SdlEventBridge.cs`
   - 添加输入事件分发（键盘、鼠标、游戏手柄）

3. `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ViewportAvaloniaView.cs`
   - 订阅 SDL 输入事件
