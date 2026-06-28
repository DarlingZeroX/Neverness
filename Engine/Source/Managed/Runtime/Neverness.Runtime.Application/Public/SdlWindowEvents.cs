// Neverness.Runtime.Application — 窗口事件分发器。
// 持有单个 SdlWindow 的所有事件回调（窗口/输入/拖放）。
// 由 SdlEventBridge 路由调用，外部不应直接调用 Dispatch。

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 窗口事件分发器。
/// 每个 SdlWindow 持有一个实例，通过 SdlWindow.Events 访问。
/// SdlEventBridge 根据 SDL_Event.windowID 路由到对应窗口的 Events.Dispatch。
/// </summary>
public sealed class SdlWindowEvents
{
    // ── 窗口事件 ──

    /// <summary>窗口大小变更 (width, height)。</summary>
    public event Action<int, int>? OnResized;

    /// <summary>窗口像素尺寸变更 (width, height)。</summary>
    public event Action<int, int>? OnPixelSizeChanged;

    /// <summary>窗口关闭请求。</summary>
    public event Action? OnCloseRequested;

    /// <summary>窗口获得/失去焦点 (gained)。</summary>
    public event Action<bool>? OnFocusChanged;

    /// <summary>窗口最小化/恢复 (minimized)。</summary>
    public event Action<bool>? OnMinimizedChanged;

    /// <summary>窗口移动 (x, y)。</summary>
    public event Action<int, int>? OnMoved;

    /// <summary>窗口显示缩放变化 (scale)。</summary>
    public event Action<float>? OnDisplayScaleChanged;

    // ── 输入事件 ──

    /// <summary>键盘按下。</summary>
    public event Action<SDL.SDL_KeyboardEvent>? OnKeyDown;

    /// <summary>键盘释放。</summary>
    public event Action<SDL.SDL_KeyboardEvent>? OnKeyUp;

    /// <summary>鼠标按钮按下。</summary>
    public event Action<SDL.SDL_MouseButtonEvent>? OnMouseButtonDown;

    /// <summary>鼠标按钮释放。</summary>
    public event Action<SDL.SDL_MouseButtonEvent>? OnMouseButtonUp;

    /// <summary>鼠标移动。</summary>
    public event Action<SDL.SDL_MouseMotionEvent>? OnMouseMotion;

    /// <summary>鼠标滚轮。</summary>
    public event Action<SDL.SDL_MouseWheelEvent>? OnMouseWheel;

    /// <summary>手柄按钮按下。</summary>
    public event Action<SDL.SDL_GamepadButtonEvent>? OnGamepadButtonDown;

    /// <summary>手柄按钮释放。</summary>
    public event Action<SDL.SDL_GamepadButtonEvent>? OnGamepadButtonUp;

    /// <summary>手柄摇杆。</summary>
    public event Action<SDL.SDL_GamepadAxisEvent>? OnGamepadAxis;

    // ── 拖放/文本事件 ──

    /// <summary>文件拖放 (filePath)。</summary>
    public event Action<string>? OnDropFile;

    /// <summary>文本拖放 (text)。</summary>
    public event Action<string>? OnDropText;

    /// <summary>文本输入 (text)。</summary>
    public event Action<string>? OnTextInput;

    /// <summary>
    /// 统一事件分发入口。由 SdlEventBridge 调用，外部不应直接调用。
    /// </summary>
    internal void Dispatch(SDL.SDL_Event e)
    {
        //Console.WriteLine(e.Type.ToString());
        switch (e.Type)
        {
            // ── 窗口事件 ──
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESIZED:
                OnResized?.Invoke(e.window.data1, e.window.data2);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                OnPixelSizeChanged?.Invoke(e.window.data1, e.window.data2);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                OnCloseRequested?.Invoke();
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_GAINED:
                OnFocusChanged?.Invoke(true);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_LOST:
                OnFocusChanged?.Invoke(false);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_MINIMIZED:
                OnMinimizedChanged?.Invoke(true);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESTORED:
                OnMinimizedChanged?.Invoke(false);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_MOVED:
                OnMoved?.Invoke(e.window.data1, e.window.data2);
                break;
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
                OnDisplayScaleChanged?.Invoke(e.window.data1 / 1000f);
                break;

            // ── 键盘事件 ──
            case SDL.SDL_EventType.SDL_EVENT_KEY_DOWN:
                OnKeyDown?.Invoke(e.key);
                break;
            case SDL.SDL_EventType.SDL_EVENT_KEY_UP:
                OnKeyUp?.Invoke(e.key);
                break;

            // ── 鼠标事件 ──
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN:
                OnMouseButtonDown?.Invoke(e.button);
                break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP:
                OnMouseButtonUp?.Invoke(e.button);
                break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION:
                OnMouseMotion?.Invoke(e.motion);
                break;
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL:
                OnMouseWheel?.Invoke(e.wheel);
                break;

            // ── 手柄事件 ──
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                OnGamepadButtonDown?.Invoke(e.gbutton);
                break;
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP:
                OnGamepadButtonUp?.Invoke(e.gbutton);
                break;
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION:
                OnGamepadAxis?.Invoke(e.gaxis);
                break;

            // ── 拖放事件 ──
            case SDL.SDL_EventType.SDL_EVENT_DROP_FILE:
                OnDropFile?.Invoke(e.drop.GetData() ?? string.Empty);
                break;
            case SDL.SDL_EventType.SDL_EVENT_DROP_TEXT:
                OnDropText?.Invoke(e.drop.GetData() ?? string.Empty);
                break;

            // ── 文本输入 ──
            case SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT:
                OnTextInput?.Invoke(e.text.GetText() ?? string.Empty);
                break;
        }
    }
}
