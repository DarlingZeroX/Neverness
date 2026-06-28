# Gameplay 输入事件路由计划

> **创建日期**: 2026-06-28
> **状态**: 待审查（已确认全部问题）

---

## 1. 现状

```
Neverness.Gameplay
├── Input/IInputProvider.cs    — 输入提供者接口（无实现）
├── Input/Input.cs             — 静态 API，委托给 IInputProvider
├── Input/KeyCode.cs           — 按键枚举（ASCII 值，与 SDL 不完全对齐）
├── Input/MouseButton.cs       — 鼠标按键枚举
└── Context/GameplayContext.cs — 持有 InputProvider，Initialize() 时注入

Neverness.Runtime.Application
├── Private/SdlEventBridge.cs  — SDL 事件泵 + 路由到 SdlWindow.Events
└── Public/SdlWindowEvents.cs  — 窗口事件分发器

问题：
- IInputProvider 无实现，Input.GetXxx() 全部返回默认值
- SdlWindowEvents 输入事件无人订阅
- NativeEventPump (C++ NNEvent) 已废弃
```

---

## 2. 架构变更

### 2.1 依赖方向

```
Neverness.Gameplay → Neverness.Runtime.Application (新增)
Neverness.Gameplay → Runtime.Scene, Runtime.Engine, Runtime.RuntimeLoop (已有)
```

### 2.2 IInputProvider 下沉到 Runtime.Application

**移动文件** (Gameplay → Runtime.Application/Public/):
- `IInputProvider.cs`
- `KeyCode.cs`
- `MouseButton.cs`

**Neverness.Gameplay.csproj** 新增:
```xml
<ProjectReference Include="..\..\Runtime\Neverness.Runtime.Application\Neverness.Runtime.Application.csproj" />
```

**Gameplay 中的** `Input.cs`、`GameplayContext.cs` 添加 `using Neverness.Runtime.Application;`。

---

## 3. KeyCode 映射

### 3.1 值差异

| 按键 | Gameplay KeyCode | SDL3 SDL_Keycode | 差异 |
|------|-----------------|-------------------|------|
| A-Z | 65-90 (大写 ASCII) | 97-122 (小写 ASCII) | 需要 +32 |
| 0-9 | 48-57 | 48-57 | 一致 |
| F1-F12 | 282-293 | 0x4000003A 起 | 完全不同 |
| 方向键 | 262-265 | 0x4000004F 起 | 完全不同 |
| 修饰键 | 340-347 | 0x400000E0 起 | 完全不同 |
| Space/Enter/Tab | 32/257/258 | 32/13/9 | 部分不同 |

### 3.2 映射方案

SDL3 特殊键使用 `SDLK_SCANCODE_MASK = 0x40000000`，通过 `SDL_Scancode + MASK` 计算。

```csharp
private static KeyCode MapKeyCode(SDL.SDL_Keycode sdlKey)
{
    var v = (int)sdlKey;

    // 字母键：SDL 用小写(97-122)，KeyCode 用大写(65-90)
    if (v >= 97 && v <= 122)
        return (KeyCode)(v - 32);

    // 数字键：直接对齐
    if (v >= 48 && v <= 57)
        return (KeyCode)v;

    // 特殊键：查表
    return v switch
    {
        13  => KeyCode.Enter,      // SDL: \r
        32  => KeyCode.Space,
        9   => KeyCode.Tab,
        8   => KeyCode.Backspace,
        127 => KeyCode.Delete,
        27  => KeyCode.Escape,
        // 方向键 (SDL3 scancode + mask)
        0x4000004F => KeyCode.Right,
        0x40000050 => KeyCode.Left,
        0x40000051 => KeyCode.Down,
        0x40000052 => KeyCode.Up,
        // 功能键
        0x4000003A => KeyCode.F1,
        0x4000003B => KeyCode.F2,
        0x4000003C => KeyCode.F3,
        0x4000003D => KeyCode.F4,
        0x4000003E => KeyCode.F5,
        0x4000003F => KeyCode.F6,
        0x40000040 => KeyCode.F7,
        0x40000041 => KeyCode.F8,
        0x40000042 => KeyCode.F9,
        0x40000043 => KeyCode.F10,
        0x40000044 => KeyCode.F11,
        0x40000045 => KeyCode.F12,
        // 修饰键
        0x400000E0 => KeyCode.LeftControl,
        0x400000E1 => KeyCode.LeftShift,
        0x400000E2 => KeyCode.LeftAlt,
        0x400000E3 => KeyCode.LeftSuper,
        0x400000E4 => KeyCode.RightControl,
        0x400000E5 => KeyCode.RightShift,
        0x400000E6 => KeyCode.RightAlt,
        0x400000E7 => KeyCode.RightSuper,
        // 特殊键
        0x40000048 => KeyCode.Pause,
        0x40000049 => KeyCode.Insert,
        0x4000004A => KeyCode.Home,
        0x4000004B => KeyCode.PageUp,
        0x4000004D => KeyCode.End,
        0x4000004E => KeyCode.PageDown,
        // 符号键 (SDL 用 ASCII)
        45  => KeyCode.Minus,
        61  => KeyCode.Equals,
        91  => KeyCode.LeftBracket,
        93  => KeyCode.RightBracket,
        92  => KeyCode.Backslash,
        59  => KeyCode.Semicolon,
        39  => KeyCode.Quote,
        44  => KeyCode.Comma,
        46  => KeyCode.Period,
        47  => KeyCode.Slash,
        96  => KeyCode.Grave,
        _   => KeyCode.None,
    };
}
```

---

## 4. 轴映射系统

### 4.1 设计

```csharp
/// <summary>轴映射定义。</summary>
public sealed class AxisMapping
{
    /// <summary>正方向键。</summary>
    public KeyCode PositiveKey { get; init; }
    /// <summary>负方向键。</summary>
    public KeyCode NegativeKey { get; init; }
    /// <summary>灵敏度。</summary>
    public float Sensitivity { get; init; } = 3f;
    /// <summary>是否为原始值（无平滑）。</summary>
    public bool Raw { get; init; }
}
```

### 4.2 默认轴

```csharp
private static readonly Dictionary<string, AxisMapping> s_defaultAxes = new()
{
    ["Horizontal"] = new AxisMapping
    {
        PositiveKey = KeyCode.D,
        NegativeKey = KeyCode.A,
    },
    ["Vertical"] = new AxisMapping
    {
        PositiveKey = KeyCode.W,
        NegativeKey = KeyCode.S,
    },
    ["Mouse X"] = new AxisMapping { Raw = true },
    ["Mouse Y"] = new AxisMapping { Raw = true },
};
```

### 4.3 SdlInputProvider 中的实现

```csharp
private readonly Dictionary<string, AxisMapping> _axes;
private readonly Dictionary<string, float> _axisValues = new();

public float GetAxis(string axisName)
{
    if (!_axes.TryGetValue(axisName, out var mapping) || mapping.Raw)
        return 0f;
    return SmoothAxis(axisName, mapping);
}

public float GetAxisRaw(string axisName)
{
    if (!_axes.TryGetValue(axisName, out var mapping))
        return 0f;
    float value = 0f;
    if (_keysDown.Contains(mapping.PositiveKey)) value += 1f;
    if (_keysDown.Contains(mapping.NegativeKey)) value -= 1f;
    return value;
}

private float SmoothAxis(string axisName, AxisMapping mapping)
{
    float target = GetAxisRaw(axisName);
    float current = _axisValues.GetValueOrDefault(axisName);
    float smoothed = Mathf.MoveTowards(current, target,
        mapping.Sensitivity * _deltaTime);
    _axisValues[axisName] = smoothed;
    return smoothed;
}
```

---

## 5. SdlInputProvider 完整设计

```csharp
/// <summary>
/// SDL 输入提供者。
/// 订阅 SdlWindowEvents，维护键盘/鼠标状态。
/// 实现 IInputProvider 供 Gameplay.Input 使用。
/// </summary>
public sealed class SdlInputProvider : IInputProvider
{
    // ── 状态 ──
    private readonly HashSet<KeyCode> _keysDown = new();
    private readonly HashSet<KeyCode> _keysDownThisFrame = new();
    private readonly HashSet<KeyCode> _keysUpThisFrame = new();
    private readonly HashSet<MouseButton> _mouseDown = new();
    private readonly HashSet<MouseButton> _mouseDownThisFrame = new();
    private readonly HashSet<MouseButton> _mouseUpThisFrame = new();
    private Vector2 _mousePosition;
    private float _mouseScrollDelta;
    private float _deltaTime;
    private readonly Dictionary<string, AxisMapping> _axes;
    private readonly Dictionary<string, float> _axisValues = new();

    public SdlInputProvider(Dictionary<string, AxisMapping>? axes = null)
    {
        _axes = axes ?? DefaultAxes.Create();
    }

    /// <summary>订阅窗口事件。</summary>
    public void Attach(SdlWindowEvents events) { ... }

    /// <summary>取消订阅。</summary>
    public void Detach(SdlWindowEvents events) { ... }

    /// <summary>每帧开始时调用，清除瞬态标志。</summary>
    public void NewFrame(float deltaTime)
    {
        _keysDownThisFrame.Clear();
        _keysUpThisFrame.Clear();
        _mouseDownThisFrame.Clear();
        _mouseUpThisFrame.Clear();
        _mouseScrollDelta = 0f;
        _deltaTime = deltaTime;
    }

    // ── IInputProvider ──
    public bool GetKey(KeyCode key) => _keysDown.Contains(key);
    public bool GetKeyDown(KeyCode key) => _keysDownThisFrame.Contains(key);
    public bool GetKeyUp(KeyCode key) => _keysUpThisFrame.Contains(key);
    public bool GetMouseButton(MouseButton b) => _mouseDown.Contains(b);
    public bool GetMouseButtonDown(MouseButton b) => _mouseDownThisFrame.Contains(b);
    public bool GetMouseButtonUp(MouseButton b) => _mouseUpThisFrame.Contains(b);
    public Vector2 MousePosition => _mousePosition;
    public float MouseScrollDelta => _mouseScrollDelta;
    public float GetAxis(string name) => SmoothAxis(name);
    public float GetAxisRaw(string name) => RawAxis(name);

    // ── SDL 事件处理 ──
    private void HandleKeyDown(SDL.SDL_KeyboardEvent e) { ... }
    private void HandleKeyUp(SDL.SDL_KeyboardEvent e) { ... }
    private void HandleMouseButtonDown(SDL.SDL_MouseButtonEvent e) { ... }
    private void HandleMouseButtonUp(SDL.SDL_MouseButtonEvent e) { ... }
    private void HandleMouseMotion(SDL.SDL_MouseMotionEvent e) { ... }
    private void HandleMouseWheel(SDL.SDL_MouseWheelEvent e) { ... }

    // ── 映射 ──
    private static KeyCode MapKeyCode(SDL.SDL_Keycode sdlKey) { ... }  // §3.2
    private static MouseButton MapMouseButton(byte sdlButton) { ... }
}
```

---

## 6. ScriptEditorModule 改动

```csharp
// 改动前:
public static void EnterPlayMode()
{
    _gameplayContext = new GameplayContext();
    _gameplayContext.Initialize();
}

// 改动后:
public static void EnterPlayMode(IInputProvider inputProvider)
{
    _gameplayContext = new GameplayContext();
    _gameplayContext.InputProvider = inputProvider;
    _gameplayContext.Initialize();
}
```

---

## 7. EditorApplicationRunner 改动

```csharp
private static SdlInputProvider? s_inputProvider;

public static void Install(SdlWindow window)
{
    // ...

    // 创建输入提供者并订阅主窗口事件
    s_inputProvider = new SdlInputProvider();
    s_inputProvider.Attach(window.Events);

    // ...
}

// 主循环:
while (ApplicationHost.PumpEvents())
{
    s_inputProvider?.NewFrame(deltaTime);
    // ...
    if (enteringPlayMode)
        ScriptEditorModule.EnterPlayMode(s_inputProvider!);
}
```

---

## 8. EditorEventPump — 替换 NativeEventPump

```csharp
// 改动前:
public EditorEventPump(NativeEventPump nativePump, IEditorEventBus editorEvents)

// 改动后:
public EditorEventPump(SdlWindowEvents windowEvents, IEditorEventBus editorEvents)
{
    _windowEvents = windowEvents;
    _windowEvents.OnDropFile += HandleDropFile;
    _windowEvents.OnCloseRequested += HandleWindowClose;
    _windowEvents.OnResized += HandleWindowResize;
    _windowEvents.OnFocusChanged += HandleFocusChanged;
}
```

---

## 9. 事件流总览

```
SDL_PollEvent
    ▼
SdlEventBridge.PumpEvents()
    ├── OnEvent?.Invoke(e)             ← ImGui 原始订阅
    └── ExtractWindowId → SdlWindow.Events.Dispatch(e)
            ├── SdlInputProvider        ← Gameplay 输入
            │   ├── HandleKeyDown → _keysDown + MapKeyCode
            │   ├── HandleMouseMotion → _mousePosition
            │   └── Input.GetXxx() 读取
            └── EditorEventPump         ← Editor 系统事件
                ├── OnCloseRequested → QuitRequested
                └── OnDropFile → AssetImport
```

---

## 10. 涉及文件

| 文件 | 操作 |
|------|------|
| `Gameplay/Neverness.Gameplay.csproj` | 修改：新增引用 Runtime.Application |
| `Gameplay/Input/IInputProvider.cs` | 删除：移到 Runtime.Application |
| `Gameplay/Input/KeyCode.cs` | 删除：移到 Runtime.Application |
| `Gameplay/Input/MouseButton.cs` | 删除：移到 Runtime.Application |
| `Gameplay/Input/Input.cs` | 修改：添加 using |
| `Gameplay/Context/GameplayContext.cs` | 修改：添加 using |
| **新建** `Runtime.Application/Public/IInputProvider.cs` | 从 Gameplay 移入 |
| **新建** `Runtime.Application/Public/KeyCode.cs` | 从 Gameplay 移入 |
| **新建** `Runtime.Application/Public/MouseButton.cs` | 从 Gameplay 移入 |
| **新建** `Runtime.Application/Public/AxisMapping.cs` | 轴映射定义 |
| **新建** `Runtime.Application/Public/SdlInputProvider.cs` | SDL 实现 |
| `Editor.Script/Public/ScriptEditorModule.cs` | 修改：EnterPlayMode(IInputProvider) |
| `NevernessEditor/EditorApplicationRunner.cs` | 修改：创建 + 注入 + NewFrame |
| `NevernessEditor/EditorEventPump.cs` | 修改：替换 NativeEventPump |

---

## 11. 执行步骤

| Step | 改动 |
|------|------|
| 1 | 移动 IInputProvider/KeyCode/MouseButton 到 Runtime.Application |
| 2 | 新建 AxisMapping + DefaultAxes |
| 3 | 新建 SdlInputProvider（含 KeyCode 映射 + 轴映射） |
| 4 | 更新 Gameplay.csproj + using |
| 5 | 修改 ScriptEditorModule.EnterPlayMode |
| 6 | 修改 EditorApplicationRunner |
| 7 | 替换 EditorEventPump |
