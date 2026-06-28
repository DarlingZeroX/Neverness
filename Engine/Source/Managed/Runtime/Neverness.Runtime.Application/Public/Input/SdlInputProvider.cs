// Neverness.Runtime.Application — SDL 输入提供者。
// 订阅 SdlWindowEvents，维护键盘/鼠标状态。
// 实现 IInputProvider 供 Gameplay.Input 使用。

using Neverness.Runtime.Application.Public;

namespace Neverness.Runtime.Application;

/// <summary>
/// SDL 输入提供者。
/// 订阅 SdlWindowEvents 的输入事件，维护帧状态，
/// 通过 IInputProvider 接口供 Gameplay 层查询。
/// </summary>
public sealed class SdlInputProvider : IInputProvider
{
    // ========================================================================
    // 状态
    // ========================================================================

    /// <summary>当前帧按下的键。</summary>
    private readonly HashSet<KeyCode> _keysDown = new();

    /// <summary>本帧新按下的键（GetKeyDown）。</summary>
    private readonly HashSet<KeyCode> _keysDownThisFrame = new();

    /// <summary>本帧释放的键（GetKeyUp）。</summary>
    private readonly HashSet<KeyCode> _keysUpThisFrame = new();

    /// <summary>当前帧按下的鼠标按钮。</summary>
    private readonly HashSet<MouseButton> _mouseDown = new();

    /// <summary>本帧新按下的鼠标按钮。</summary>
    private readonly HashSet<MouseButton> _mouseDownThisFrame = new();

    /// <summary>本帧释放的鼠标按钮。</summary>
    private readonly HashSet<MouseButton> _mouseUpThisFrame = new();

    /// <summary>鼠标位置。</summary>
    private (float X, float Y) _mousePosition;

    /// <summary>鼠标滚轮增量。</summary>
    private float _mouseScrollDelta;

    /// <summary>帧间隔时间（用于轴平滑）。</summary>
    private float _deltaTime;

    /// <summary>轴映射配置。</summary>
    private readonly Dictionary<string, AxisMapping> _axes;

    /// <summary>轴当前平滑值。</summary>
    private readonly Dictionary<string, float> _axisValues = new();

    // ========================================================================
    // 构造
    // ========================================================================

    /// <summary>
    /// 创建 SDL 输入提供者。
    /// </summary>
    /// <param name="axes">轴映射配置。为 null 时使用默认配置（Horizontal=A/D, Vertical=W/S）。</param>
    public SdlInputProvider(Dictionary<string, AxisMapping>? axes = null)
    {
        _axes = axes ?? DefaultAxes.Create();
    }

    // ========================================================================
    // 生命周期
    // ========================================================================

    /// <summary>订阅窗口事件。</summary>
    public void Attach(SdlWindowEvents events)
    {
        events.OnKeyDown += HandleKeyDown;
        events.OnKeyUp += HandleKeyUp;
        events.OnMouseButtonDown += HandleMouseButtonDown;
        events.OnMouseButtonUp += HandleMouseButtonUp;
        events.OnMouseMotion += HandleMouseMotion;
        events.OnMouseWheel += HandleMouseWheel;
    }

    /// <summary>取消订阅窗口事件。</summary>
    public void Detach(SdlWindowEvents events)
    {
        events.OnKeyDown -= HandleKeyDown;
        events.OnKeyUp -= HandleKeyUp;
        events.OnMouseButtonDown -= HandleMouseButtonDown;
        events.OnMouseButtonUp -= HandleMouseButtonUp;
        events.OnMouseMotion -= HandleMouseMotion;
        events.OnMouseWheel -= HandleMouseWheel;
    }

    /// <summary>
    /// 每帧开始时调用，清除瞬态标志。
    /// 必须在 ApplicationHost.PumpEvents() 之后、Gameplay Tick 之前调用。
    /// </summary>
    /// <param name="deltaTime">帧间隔时间。</param>
    public void NewFrame(float deltaTime)
    {
        _keysDownThisFrame.Clear();
        _keysUpThisFrame.Clear();
        _mouseDownThisFrame.Clear();
        _mouseUpThisFrame.Clear();
        _mouseScrollDelta = 0f;
        _deltaTime = deltaTime;
    }

    // ========================================================================
    // IInputProvider — 键盘
    // ========================================================================

    /// <inheritdoc/>
    public bool GetKey(KeyCode key) => _keysDown.Contains(key);

    /// <inheritdoc/>
    public bool GetKeyDown(KeyCode key) => _keysDownThisFrame.Contains(key);

    /// <inheritdoc/>
    public bool GetKeyUp(KeyCode key) => _keysUpThisFrame.Contains(key);

    // ========================================================================
    // IInputProvider — 鼠标
    // ========================================================================

    /// <inheritdoc/>
    public bool GetMouseButton(MouseButton button) => _mouseDown.Contains(button);

    /// <inheritdoc/>
    public bool GetMouseButtonDown(MouseButton button) => _mouseDownThisFrame.Contains(button);

    /// <inheritdoc/>
    public bool GetMouseButtonUp(MouseButton button) => _mouseUpThisFrame.Contains(button);

    /// <inheritdoc/>
    public (float X, float Y) MousePosition => _mousePosition;

    /// <inheritdoc/>
    public float MouseScrollDelta => _mouseScrollDelta;

    // ========================================================================
    // IInputProvider — 轴
    // ========================================================================

    /// <inheritdoc/>
    public float GetAxis(string axisName)
    {
        if (!_axes.TryGetValue(axisName, out var mapping))
            return 0f;

        // 原始轴直接返回瞬时值
        if (mapping.Raw)
            return GetAxisRaw(axisName);

        // 平滑轴：渐进接近目标值
        float target = GetAxisRaw(axisName);
        float current = _axisValues.GetValueOrDefault(axisName);
        float maxDelta = mapping.Sensitivity * _deltaTime;
        float smoothed = MoveTowards(current, target, maxDelta);
        _axisValues[axisName] = smoothed;
        return smoothed;
    }

    /// <inheritdoc/>
    public float GetAxisRaw(string axisName)
    {
        if (!_axes.TryGetValue(axisName, out var mapping))
            return 0f;

        float value = 0f;
        if (_keysDown.Contains(mapping.PositiveKey)) value += 1f;
        if (_keysDown.Contains(mapping.NegativeKey)) value -= 1f;
        return value;
    }

    // ========================================================================
    // SDL 事件处理
    // ========================================================================

    private void HandleKeyDown(SDL.SDL_KeyboardEvent e)
    {
        var key = MapKeyCode(e.key);
        if (key != KeyCode.None && _keysDown.Add(key))
        {
            _keysDownThisFrame.Add(key);
        }
    }

    private void HandleKeyUp(SDL.SDL_KeyboardEvent e)
    {
        var key = MapKeyCode(e.key);
        if (key != KeyCode.None && _keysDown.Remove(key))
        {
            _keysUpThisFrame.Add(key);
        }
    }

    private void HandleMouseButtonDown(SDL.SDL_MouseButtonEvent e)
    {
        var button = MapMouseButton((int)e.button);
        if (_mouseDown.Add(button))
        {
            _mouseDownThisFrame.Add(button);
        }
    }

    private void HandleMouseButtonUp(SDL.SDL_MouseButtonEvent e)
    {
        var button = MapMouseButton((int)e.button);
        if (_mouseDown.Remove(button))
        {
            _mouseUpThisFrame.Add(button);
        }
    }

    private void HandleMouseMotion(SDL.SDL_MouseMotionEvent e)
    {
        _mousePosition = (e.x, e.y);
    }

    private void HandleMouseWheel(SDL.SDL_MouseWheelEvent e)
    {
        _mouseScrollDelta += e.y;
    }

    // ========================================================================
    // 映射
    // ========================================================================

    /// <summary>
    /// SDL3 keycode → KeyCode 映射。
    /// SDL3 字母键用小写 ASCII (97-122)，KeyCode 用大写 (65-90)。
    /// SDL3 特殊键用 scancode | 0x40000000。
    /// </summary>
    private static KeyCode MapKeyCode(SDL.SDL_Keycode sdlKey)
    {
        var v = (int)sdlKey;

        // 字母键：SDL 用小写(97-122)，KeyCode 用大写(65-90)
        if (v >= 97 && v <= 122)
            return (KeyCode)(v - 32);

        // 数字键和 ASCII 符号键：直接对齐
        if (v >= 32 && v <= 126 && Enum.IsDefined(typeof(KeyCode), v))
            return (KeyCode)v;

        // 特殊键：查表
        return v switch
        {
            // 功能键 (SDL3: scancode | 0x40000000)
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

            // 方向键
            0x4000004F => KeyCode.Right,
            0x40000050 => KeyCode.Left,
            0x40000051 => KeyCode.Down,
            0x40000052 => KeyCode.Up,

            // 特殊键
            0x40000048 => KeyCode.Pause,
            0x40000049 => KeyCode.Insert,
            0x4000004A => KeyCode.Home,
            0x4000004B => KeyCode.PageUp,
            0x4000004D => KeyCode.End,
            0x4000004E => KeyCode.PageDown,

            // 修饰键
            0x400000E0 => KeyCode.LeftControl,
            0x400000E1 => KeyCode.LeftShift,
            0x400000E2 => KeyCode.LeftAlt,
            0x400000E3 => KeyCode.LeftSuper,
            0x400000E4 => KeyCode.RightControl,
            0x400000E5 => KeyCode.RightShift,
            0x400000E6 => KeyCode.RightAlt,
            0x400000E7 => KeyCode.RightSuper,

            _ => KeyCode.None,
        };
    }

    /// <summary>SDL 鼠标按钮 → MouseButton 映射。</summary>
    private static MouseButton MapMouseButton(int sdlButton) => sdlButton switch
    {
        
        SDL.SDL3.SDL_BUTTON_LEFT => MouseButton.Left,
        SDL.SDL3.SDL_BUTTON_MIDDLE => MouseButton.Middle,
        SDL.SDL3.SDL_BUTTON_RIGHT => MouseButton.Right,
        SDL.SDL3.SDL_BUTTON_X1 => MouseButton.Back,
        SDL.SDL3.SDL_BUTTON_X2 => MouseButton.Forward,
        _ => MouseButton.Left,
    };

    // ========================================================================
    // 工具方法
    // ========================================================================

    /// <summary>向目标值移动（类似 Unity Mathf.MoveTowards）。</summary>
    private static float MoveTowards(float current, float target, float maxDelta)
    {
        if (MathF.Abs(target - current) <= maxDelta)
            return target;
        return current + MathF.Sign(target - current) * maxDelta;
    }
}
