// Neverness.Runtime.Application — SDL3 事件泵。
// 职责：泵送 SDL_Event + 路由到对应 SdlWindow.Events.Dispatch。
// 不持有事件逻辑，只做分发。

using Neverness.Runtime.Application.Public;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// SDL3 事件泵。
/// 泵送 SDL_Event，根据 windowID 路由到对应 SdlWindow 的事件分发器。
/// 只有 QUIT/TERMINATING 作为全局事件处理（无 windowID）。
/// </summary>
internal static unsafe class SdlEventBridge
{
    private static bool s_shouldQuit;

    /// <summary>是否收到退出请求。</summary>
    public static bool ShouldQuit => s_shouldQuit;

    /// <summary>
    /// 所有 SDL 事件的原始广播（ImGui 等全局订阅者使用）。
    /// 在路由到窗口之前调用，保证 ImGui 最先收到事件。
    /// </summary>
    public static event Action<SDL.SDL_Event>? OnEvent;

    /// <summary>
    /// 泵送事件；返回 false 表示应退出主循环。
    /// 每帧调用一次。
    /// </summary>
    public static bool PumpEvents()
    {
        SDL.SDL_Event e;
        while (SDL.SDL3.SDL_PollEvent(&e))
        {
            // 1. 原始广播（ImGui 等）
            OnEvent?.Invoke(e);

            // 2. 全局事件（无 windowID）
            if (e.Type is SDL.SDL_EventType.SDL_EVENT_QUIT
                        or SDL.SDL_EventType.SDL_EVENT_TERMINATING)
            {
                s_shouldQuit = true;
                continue;
            }

            // 3. 路由到对应窗口
            var windowId = ExtractWindowId(e);
            if (windowId != 0)
            {
                var window = SdlWindowManager.Resolve(new WindowHandle(windowId));
                window?.Events.Dispatch(e);
            }
        }

        return !s_shouldQuit;
    }

    /// <summary>重置退出标志（用于重新进入主循环）。</summary>
    public static void ResetQuitFlag()
    {
        s_shouldQuit = false;
    }

    /// <summary>
    /// 从 SDL_Event 中提取 windowID。
    /// 不同事件类型的 windowID 字段在 union 中偏移不同，必须按类型访问。
    /// </summary>
    private static uint ExtractWindowId(SDL.SDL_Event e) => e.Type switch
    {
        // 窗口事件
        >= SDL.SDL_EventType.SDL_EVENT_WINDOW_FIRST
        and <= SDL.SDL_EventType.SDL_EVENT_WINDOW_LAST
            => (uint)e.window.windowID,

        // 键盘
        SDL.SDL_EventType.SDL_EVENT_KEY_DOWN
        or SDL.SDL_EventType.SDL_EVENT_KEY_UP
            => (uint)e.key.windowID,

        // 鼠标按钮
        SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN
        or SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP
            => (uint)e.button.windowID,

        // 鼠标移动
        SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION
            => (uint)e.motion.windowID,

        // 鼠标滚轮
        SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL
            => (uint)e.wheel.windowID,

        // 手柄（无 windowID，路由到焦点窗口）
        SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN
        or SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP
        or SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION
            => GetFocusedWindowId(),

        // 拖放
        SDL.SDL_EventType.SDL_EVENT_DROP_FILE
        or SDL.SDL_EventType.SDL_EVENT_DROP_TEXT
            => (uint)e.drop.windowID,

        // 文本输入
        SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT
            => (uint)e.text.windowID,

        _ => 0,
    };

    /// <summary>获取当前焦点窗口的 SDL_WindowID（手柄事件无 windowID）。</summary>
    private static uint GetFocusedWindowId()
    {
        var focusedWindow = SDL.SDL3.SDL_GetKeyboardFocus();
        return focusedWindow != null
            ? (uint)SDL.SDL3.SDL_GetWindowID(focusedWindow)
            : 0;
    }
}
