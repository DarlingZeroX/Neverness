using System.Runtime.InteropServices;
using RmlUiNet;
using RmlUiNet.Input;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI SDL3 平台层。
///
/// 职责：
/// - SystemInterface 实现（时间、光标、剪贴板、键盘）
/// - SDL 事件转换为 RmlUi 输入
///
/// 对应 C++ RmlUi_Platform_SDL.h/.cpp
/// </summary>
public sealed unsafe class RmlSdlPlatform : IDisposable
{
    #region 字段

    /// <summary>性能计数器起始值。</summary>
    private readonly ulong _performanceStart;

    /// <summary>性能计数器频率。</summary>
    private readonly double _performanceFrequency;

    /// <summary>SDL 窗口指针。</summary>
    private SDL.SDL_Window* _window;

    /// <summary>鼠标光标缓存。</summary>
    private SDL.SDL_Cursor* _cursorDefault;
    private SDL.SDL_Cursor* _cursorMove;
    private SDL.SDL_Cursor* _cursorPointer;
    private SDL.SDL_Cursor* _cursorResize;
    private SDL.SDL_Cursor* _cursorCross;
    private SDL.SDL_Cursor* _cursorText;
    private SDL.SDL_Cursor* _cursorUnavailable;

    #endregion

    /// <summary>
    /// 创建 SDL 平台层。
    /// </summary>
    /// <param name="window">SDL 窗口指针。</param>
    public RmlSdlPlatform(SDL.SDL_Window* window)
    {
        _window = window;

        // 初始化性能计数器
        _performanceStart = SDL.SDL3.SDL_GetPerformanceCounter();
        _performanceFrequency = SDL.SDL3.SDL_GetPerformanceFrequency();

        // 创建鼠标光标
        _cursorDefault = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_DEFAULT);
        _cursorMove = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_MOVE);
        _cursorPointer = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_POINTER);
        _cursorResize = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_NWSE_RESIZE);
        _cursorCross = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_CROSSHAIR);
        _cursorText = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_TEXT);
        _cursorUnavailable = SDL.SDL3.SDL_CreateSystemCursor(SDL.SDL_SystemCursor.SDL_SYSTEM_CURSOR_NOT_ALLOWED);
    }

    #region SystemInterface 回调

    /// <summary>
    /// 获取经过的时间（秒）。
    /// </summary>
    public double GetElapsedTime()
    {
        return (double)(SDL.SDL3.SDL_GetPerformanceCounter() - _performanceStart) / _performanceFrequency;
    }

    /// <summary>
    /// 设置鼠标光标。
    /// </summary>
    public void SetMouseCursor(string cursorName)
    {
        SDL.SDL_Cursor* cursor = null;

        if (string.IsNullOrEmpty(cursorName) || cursorName == "arrow")
            cursor = _cursorDefault;
        else if (cursorName == "move")
            cursor = _cursorMove;
        else if (cursorName == "pointer")
            cursor = _cursorPointer;
        else if (cursorName == "resize")
            cursor = _cursorResize;
        else if (cursorName == "cross")
            cursor = _cursorCross;
        else if (cursorName == "text")
            cursor = _cursorText;
        else if (cursorName == "unavailable")
            cursor = _cursorUnavailable;
        else if (cursorName.StartsWith("rmlui-scroll"))
            cursor = _cursorMove;

        if (cursor != null)
            SDL.SDL3.SDL_SetCursor(cursor);
    }

    /// <summary>
    /// 设置剪贴板文本。
    /// </summary>
    public void SetClipboardText(string text)
    {
        SDL.SDL3.SDL_SetClipboardText(text);
    }

    /// <summary>
    /// 获取剪贴板文本。
    /// </summary>
    public string GetClipboardText()
    {
        return SDL.SDL3.SDL_GetClipboardText() ?? string.Empty;
    }

    /// <summary>
    /// 激活键盘（显示输入法候选框）。
    /// </summary>
    public void ActivateKeyboard(float caretX, float caretY, float lineHeight)
    {
        if (_window != null)
        {
            var rect = new SDL.SDL_Rect
            {
                x = (int)caretX,
                y = (int)caretY,
                w = 1,
                h = (int)lineHeight
            };
            SDL.SDL3.SDL_SetTextInputArea(_window, &rect, 0);
            SDL.SDL3.SDL_StartTextInput(_window);
        }
    }

    /// <summary>
    /// 停用键盘。
    /// </summary>
    public void DeactivateKeyboard()
    {
        if (_window != null)
        {
            SDL.SDL3.SDL_StopTextInput(_window);
        }
    }

    #endregion

    #region SDL 事件处理

    /// <summary>
    /// 处理 SDL 事件，转换为 RmlUi 输入。
    /// </summary>
    /// <param name="context">RmlUi Context。</param>
    /// <param name="event">SDL 事件。</param>
    /// <returns>如果事件仍需传播返回 true，被 Context 处理返回 false。</returns>
    public bool HandleEvent(Context context, SDL.SDL_Event @event)
    {
        bool result = true;

        switch (@event.Type)
        {
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION:
            {
                float pixelDensity = SDL.SDL3.SDL_GetWindowPixelDensity(_window);
                int x = (int)(@event.motion.x * pixelDensity);
                int y = (int)(@event.motion.y * pixelDensity);
                result = context.ProcessMouseMove(x, y, GetKeyModifierState());
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                result = context.ProcessMouseButtonDown(
                    ConvertMouseButton(@event.button.button),
                    GetKeyModifierState());
                SDL.SDL3.SDL_CaptureMouse(true);
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP:
            {
                SDL.SDL3.SDL_CaptureMouse(false);
                result = context.ProcessMouseButtonUp(
                    ConvertMouseButton(@event.button.button),
                    GetKeyModifierState());
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL:
            {
                result = context.ProcessMouseWheel(
                    new Vector2f(0, -@event.wheel.y),
                    GetKeyModifierState());
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_KEY_DOWN:
            {
                var key = ConvertKey(@event.key.key);
                result = context.ProcessKeyDown(key, GetKeyModifierState());

                // 回车键需要额外发送文本输入
                if (@event.key.key == SDL.SDL_Keycode.SDLK_RETURN ||
                    @event.key.key == SDL.SDL_Keycode.SDLK_KP_ENTER)
                {
                    result &= context.ProcessTextInput("\n");
                }
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_KEY_UP:
            {
                var key = ConvertKey(@event.key.key);
                result = context.ProcessKeyUp(key, GetKeyModifierState());
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT:
            {
                var textStr = @event.text.GetText();
                if (!string.IsNullOrEmpty(textStr))
                {
                    result = context.ProcessTextInput(textStr);
                }
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            {
                int w = @event.window.data1;
                int h = @event.window.data2;
                context.SetDimensions(w, h);
                break;
            }

            case SDL.SDL_EventType.SDL_EVENT_WINDOW_MOUSE_LEAVE:
            {
                context.ProcessMouseLeave();
                break;
            }

            // 注意：SetDensityIndependentPixelRatio 在 RmlUi.Net 中未暴露
            // 如需支持 DPI 缩放，需要扩展 RmlUi.Net
            case SDL.SDL_EventType.SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            {
                // TODO: 需要在 RmlUi.Net 中暴露此 API
                // float displayScale = SDL.SDL3.SDL_GetWindowDisplayScale(_window);
                // context.SetDensityIndependentPixelRatio(displayScale);
                break;
            }
        }

        return result;
    }

    #endregion

    #region 键盘转换

    /// <summary>
    /// 获取当前按键修饰符状态。
    /// </summary>
    public static KeyModifier GetKeyModifierState()
    {
        var sdlMods = SDL.SDL3.SDL_GetModState();
        KeyModifier result = KeyModifier.None;

        if ((sdlMods & SDL.SDL_Keymod.SDL_KMOD_CTRL) != 0)
            result |= KeyModifier.KM_CTRL;
        if ((sdlMods & SDL.SDL_Keymod.SDL_KMOD_SHIFT) != 0)
            result |= KeyModifier.KM_SHIFT;
        if ((sdlMods & SDL.SDL_Keymod.SDL_KMOD_ALT) != 0)
            result |= KeyModifier.KM_ALT;
        if ((sdlMods & SDL.SDL_Keymod.SDL_KMOD_NUM) != 0)
            result |= KeyModifier.KM_NUMLOCK;
        if ((sdlMods & SDL.SDL_Keymod.SDL_KMOD_CAPS) != 0)
            result |= KeyModifier.KM_CAPSLOCK;

        return result;
    }

    /// <summary>
    /// SDL 按键转换为 RmlUi 按键标识。
    /// </summary>
    public static KeyIdentifier ConvertKey(SDL.SDL_Keycode sdlKey)
    {
        return sdlKey switch
        {
            SDL.SDL_Keycode.SDLK_UNKNOWN => KeyIdentifier.KI_UNKNOWN,
            SDL.SDL_Keycode.SDLK_ESCAPE => KeyIdentifier.KI_ESCAPE,
            SDL.SDL_Keycode.SDLK_SPACE => KeyIdentifier.KI_SPACE,

            // 数字
            SDL.SDL_Keycode.SDLK_0 => KeyIdentifier.KI_0,
            SDL.SDL_Keycode.SDLK_1 => KeyIdentifier.KI_1,
            SDL.SDL_Keycode.SDLK_2 => KeyIdentifier.KI_2,
            SDL.SDL_Keycode.SDLK_3 => KeyIdentifier.KI_3,
            SDL.SDL_Keycode.SDLK_4 => KeyIdentifier.KI_4,
            SDL.SDL_Keycode.SDLK_5 => KeyIdentifier.KI_5,
            SDL.SDL_Keycode.SDLK_6 => KeyIdentifier.KI_6,
            SDL.SDL_Keycode.SDLK_7 => KeyIdentifier.KI_7,
            SDL.SDL_Keycode.SDLK_8 => KeyIdentifier.KI_8,
            SDL.SDL_Keycode.SDLK_9 => KeyIdentifier.KI_9,

            // 字母
            SDL.SDL_Keycode.SDLK_A => KeyIdentifier.KI_A,
            SDL.SDL_Keycode.SDLK_B => KeyIdentifier.KI_B,
            SDL.SDL_Keycode.SDLK_C => KeyIdentifier.KI_C,
            SDL.SDL_Keycode.SDLK_D => KeyIdentifier.KI_D,
            SDL.SDL_Keycode.SDLK_E => KeyIdentifier.KI_E,
            SDL.SDL_Keycode.SDLK_F => KeyIdentifier.KI_F,
            SDL.SDL_Keycode.SDLK_G => KeyIdentifier.KI_G,
            SDL.SDL_Keycode.SDLK_H => KeyIdentifier.KI_H,
            SDL.SDL_Keycode.SDLK_I => KeyIdentifier.KI_I,
            SDL.SDL_Keycode.SDLK_J => KeyIdentifier.KI_J,
            SDL.SDL_Keycode.SDLK_K => KeyIdentifier.KI_K,
            SDL.SDL_Keycode.SDLK_L => KeyIdentifier.KI_L,
            SDL.SDL_Keycode.SDLK_M => KeyIdentifier.KI_M,
            SDL.SDL_Keycode.SDLK_N => KeyIdentifier.KI_N,
            SDL.SDL_Keycode.SDLK_O => KeyIdentifier.KI_O,
            SDL.SDL_Keycode.SDLK_P => KeyIdentifier.KI_P,
            SDL.SDL_Keycode.SDLK_Q => KeyIdentifier.KI_Q,
            SDL.SDL_Keycode.SDLK_R => KeyIdentifier.KI_R,
            SDL.SDL_Keycode.SDLK_S => KeyIdentifier.KI_S,
            SDL.SDL_Keycode.SDLK_T => KeyIdentifier.KI_T,
            SDL.SDL_Keycode.SDLK_U => KeyIdentifier.KI_U,
            SDL.SDL_Keycode.SDLK_V => KeyIdentifier.KI_V,
            SDL.SDL_Keycode.SDLK_W => KeyIdentifier.KI_W,
            SDL.SDL_Keycode.SDLK_X => KeyIdentifier.KI_X,
            SDL.SDL_Keycode.SDLK_Y => KeyIdentifier.KI_Y,
            SDL.SDL_Keycode.SDLK_Z => KeyIdentifier.KI_Z,

            // 符号
            SDL.SDL_Keycode.SDLK_SEMICOLON => KeyIdentifier.KI_OEM_1,
            SDL.SDL_Keycode.SDLK_PLUS => KeyIdentifier.KI_OEM_PLUS,
            SDL.SDL_Keycode.SDLK_COMMA => KeyIdentifier.KI_OEM_COMMA,
            SDL.SDL_Keycode.SDLK_MINUS => KeyIdentifier.KI_OEM_MINUS,
            SDL.SDL_Keycode.SDLK_PERIOD => KeyIdentifier.KI_OEM_PERIOD,
            SDL.SDL_Keycode.SDLK_SLASH => KeyIdentifier.KI_OEM_2,
            SDL.SDL_Keycode.SDLK_GRAVE => KeyIdentifier.KI_OEM_3,
            SDL.SDL_Keycode.SDLK_LEFTBRACKET => KeyIdentifier.KI_OEM_4,
            SDL.SDL_Keycode.SDLK_BACKSLASH => KeyIdentifier.KI_OEM_5,
            SDL.SDL_Keycode.SDLK_RIGHTBRACKET => KeyIdentifier.KI_OEM_6,
            SDL.SDL_Keycode.SDLK_DBLAPOSTROPHE => KeyIdentifier.KI_OEM_7,

            // 小键盘
            SDL.SDL_Keycode.SDLK_KP_0 => KeyIdentifier.KI_NUMPAD0,
            SDL.SDL_Keycode.SDLK_KP_1 => KeyIdentifier.KI_NUMPAD1,
            SDL.SDL_Keycode.SDLK_KP_2 => KeyIdentifier.KI_NUMPAD2,
            SDL.SDL_Keycode.SDLK_KP_3 => KeyIdentifier.KI_NUMPAD3,
            SDL.SDL_Keycode.SDLK_KP_4 => KeyIdentifier.KI_NUMPAD4,
            SDL.SDL_Keycode.SDLK_KP_5 => KeyIdentifier.KI_NUMPAD5,
            SDL.SDL_Keycode.SDLK_KP_6 => KeyIdentifier.KI_NUMPAD6,
            SDL.SDL_Keycode.SDLK_KP_7 => KeyIdentifier.KI_NUMPAD7,
            SDL.SDL_Keycode.SDLK_KP_8 => KeyIdentifier.KI_NUMPAD8,
            SDL.SDL_Keycode.SDLK_KP_9 => KeyIdentifier.KI_NUMPAD9,
            SDL.SDL_Keycode.SDLK_KP_ENTER => KeyIdentifier.KI_NUMPADENTER,
            SDL.SDL_Keycode.SDLK_KP_MULTIPLY => KeyIdentifier.KI_MULTIPLY,
            SDL.SDL_Keycode.SDLK_KP_PLUS => KeyIdentifier.KI_ADD,
            SDL.SDL_Keycode.SDLK_KP_MINUS => KeyIdentifier.KI_SUBTRACT,
            SDL.SDL_Keycode.SDLK_KP_PERIOD => KeyIdentifier.KI_DECIMAL,
            SDL.SDL_Keycode.SDLK_KP_DIVIDE => KeyIdentifier.KI_DIVIDE,
            SDL.SDL_Keycode.SDLK_KP_EQUALS => KeyIdentifier.KI_OEM_NEC_EQUAL,

            // 控制键
            SDL.SDL_Keycode.SDLK_BACKSPACE => KeyIdentifier.KI_BACK,
            SDL.SDL_Keycode.SDLK_TAB => KeyIdentifier.KI_TAB,
            SDL.SDL_Keycode.SDLK_CLEAR => KeyIdentifier.KI_CLEAR,
            SDL.SDL_Keycode.SDLK_RETURN => KeyIdentifier.KI_RETURN,
            SDL.SDL_Keycode.SDLK_PAUSE => KeyIdentifier.KI_PAUSE,
            SDL.SDL_Keycode.SDLK_CAPSLOCK => KeyIdentifier.KI_CAPITAL,
            SDL.SDL_Keycode.SDLK_PAGEUP => KeyIdentifier.KI_PRIOR,
            SDL.SDL_Keycode.SDLK_PAGEDOWN => KeyIdentifier.KI_NEXT,
            SDL.SDL_Keycode.SDLK_END => KeyIdentifier.KI_END,
            SDL.SDL_Keycode.SDLK_HOME => KeyIdentifier.KI_HOME,
            SDL.SDL_Keycode.SDLK_LEFT => KeyIdentifier.KI_LEFT,
            SDL.SDL_Keycode.SDLK_UP => KeyIdentifier.KI_UP,
            SDL.SDL_Keycode.SDLK_RIGHT => KeyIdentifier.KI_RIGHT,
            SDL.SDL_Keycode.SDLK_DOWN => KeyIdentifier.KI_DOWN,
            SDL.SDL_Keycode.SDLK_INSERT => KeyIdentifier.KI_INSERT,
            SDL.SDL_Keycode.SDLK_DELETE => KeyIdentifier.KI_DELETE,
            SDL.SDL_Keycode.SDLK_HELP => KeyIdentifier.KI_HELP,

            // 功能键
            SDL.SDL_Keycode.SDLK_F1 => KeyIdentifier.KI_F1,
            SDL.SDL_Keycode.SDLK_F2 => KeyIdentifier.KI_F2,
            SDL.SDL_Keycode.SDLK_F3 => KeyIdentifier.KI_F3,
            SDL.SDL_Keycode.SDLK_F4 => KeyIdentifier.KI_F4,
            SDL.SDL_Keycode.SDLK_F5 => KeyIdentifier.KI_F5,
            SDL.SDL_Keycode.SDLK_F6 => KeyIdentifier.KI_F6,
            SDL.SDL_Keycode.SDLK_F7 => KeyIdentifier.KI_F7,
            SDL.SDL_Keycode.SDLK_F8 => KeyIdentifier.KI_F8,
            SDL.SDL_Keycode.SDLK_F9 => KeyIdentifier.KI_F9,
            SDL.SDL_Keycode.SDLK_F10 => KeyIdentifier.KI_F10,
            SDL.SDL_Keycode.SDLK_F11 => KeyIdentifier.KI_F11,
            SDL.SDL_Keycode.SDLK_F12 => KeyIdentifier.KI_F12,
            SDL.SDL_Keycode.SDLK_F13 => KeyIdentifier.KI_F13,
            SDL.SDL_Keycode.SDLK_F14 => KeyIdentifier.KI_F14,
            SDL.SDL_Keycode.SDLK_F15 => KeyIdentifier.KI_F15,

            // 锁定键
            SDL.SDL_Keycode.SDLK_NUMLOCKCLEAR => KeyIdentifier.KI_NUMLOCK,
            SDL.SDL_Keycode.SDLK_SCROLLLOCK => KeyIdentifier.KI_SCROLL,

            // 修饰键
            SDL.SDL_Keycode.SDLK_LSHIFT => KeyIdentifier.KI_LSHIFT,
            SDL.SDL_Keycode.SDLK_RSHIFT => KeyIdentifier.KI_RSHIFT,
            SDL.SDL_Keycode.SDLK_LCTRL => KeyIdentifier.KI_LCONTROL,
            SDL.SDL_Keycode.SDLK_RCTRL => KeyIdentifier.KI_RCONTROL,
            SDL.SDL_Keycode.SDLK_LALT => KeyIdentifier.KI_LMENU,
            SDL.SDL_Keycode.SDLK_RALT => KeyIdentifier.KI_RMENU,
            SDL.SDL_Keycode.SDLK_LGUI => KeyIdentifier.KI_LMETA,
            SDL.SDL_Keycode.SDLK_RGUI => KeyIdentifier.KI_RMETA,

            _ => KeyIdentifier.KI_UNKNOWN
        };
    }

    /// <summary>
    /// SDL 鼠标按钮转换为 RmlUi 鼠标按钮。
    /// SDL3 鼠标按钮常量：1=左键, 2=中键, 3=右键
    /// </summary>
    public static int ConvertMouseButton(byte sdlButton)
    {
        return sdlButton switch
        {
            1 => 0,  // SDL_BUTTON_LEFT -> RmlUi 左键
            2 => 1,  // SDL_BUTTON_RIGHT -> RmlUi 右键
            3 => 2,  // SDL_BUTTON_MIDDLE -> RmlUi 中键
            _ => 3
        };
    }

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_cursorDefault != null) { SDL.SDL3.SDL_DestroyCursor(_cursorDefault); _cursorDefault = null; }
        if (_cursorMove != null) { SDL.SDL3.SDL_DestroyCursor(_cursorMove); _cursorMove = null; }
        if (_cursorPointer != null) { SDL.SDL3.SDL_DestroyCursor(_cursorPointer); _cursorPointer = null; }
        if (_cursorResize != null) { SDL.SDL3.SDL_DestroyCursor(_cursorResize); _cursorResize = null; }
        if (_cursorCross != null) { SDL.SDL3.SDL_DestroyCursor(_cursorCross); _cursorCross = null; }
        if (_cursorText != null) { SDL.SDL3.SDL_DestroyCursor(_cursorText); _cursorText = null; }
        if (_cursorUnavailable != null) { SDL.SDL3.SDL_DestroyCursor(_cursorUnavailable); _cursorUnavailable = null; }
    }

    #endregion
}
