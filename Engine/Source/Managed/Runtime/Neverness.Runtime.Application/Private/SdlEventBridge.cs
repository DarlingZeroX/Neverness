// Neverness.Runtime.Application — SDL3 事件泵。
// 替代 C++ EventQueue + SDL3EventTranslator，直接分发 SDL_Event。
// 不翻译为 NNEvent，C# 代码直接使用 SDL_Event 类型。

using Neverness.Runtime.Application.Public;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// SDL3 事件泵。
/// 直接分发 SDL_Event，不翻译为 NNEvent。
/// 提供便捷回调用于常用事件（DropFile、TextInput 等）。
/// </summary>
internal static unsafe class SdlEventBridge
{
    private static bool s_shouldQuit;

    /// <summary>是否收到退出请求。</summary>
    public static bool ShouldQuit => s_shouldQuit;

    /// <summary>所有 SDL 事件的原始回调（SDL_Event* 为栈上指针，仅在回调内有效）。</summary>
    public static event Action<SDL.SDL_Event>? OnEvent;

    /// <summary>文件拖放事件 (windowId, filePath)。</summary>
    public static event Action<uint, string>? OnDropFile;

    /// <summary>文本拖放事件 (windowId, text)。</summary>
    public static event Action<uint, string>? OnDropText;

    /// <summary>文本输入事件 (windowId, text)。</summary>
    public static event Action<uint, string>? OnTextInput;

    /// <summary>窗口大小变更事件 (windowId, width, height)。</summary>
    public static event Action<uint, int, int>? OnWindowSizeChanged;

    /// <summary>窗口关闭事件 (windowId)。</summary>
    public static event Action<uint>? OnWindowClose;

    /// <summary>窗口焦点事件 (windowId, gained)。</summary>
    public static event Action<uint, bool>? OnWindowFocus;

    /// <summary>窗口最小化/恢复事件 (windowId, minimized)。</summary>
    public static event Action<uint, bool>? OnWindowMinimized;

    /// <summary>
    /// 泵送事件；返回 false 表示应退出主循环。
    /// 每帧调用一次。
    /// </summary>
    public static bool PumpEvents()
    {
        SDL.SDL_Event e;
        while (SDL.SDL3.SDL_PollEvent(&e))
        {
            // 分发原始事件
            OnEvent?.Invoke(e);

            // 提取常用事件到便捷回调
            switch (e.Type)
            {
                case SDL.SDL_EventType.SDL_EVENT_QUIT:
                case SDL.SDL_EventType.SDL_EVENT_TERMINATING:
                    s_shouldQuit = true;
                    break;

                case SDL.SDL_EventType.SDL_EVENT_DROP_FILE:
                    OnDropFile?.Invoke((uint)e.drop.windowID,
                        e.drop.GetData() ?? string.Empty);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_DROP_TEXT:
                    OnDropText?.Invoke((uint)e.drop.windowID,
                        e.drop.GetData() ?? string.Empty);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_TEXT_INPUT:
                    OnTextInput?.Invoke((uint)e.text.windowID,
                        e.text.GetText() ?? string.Empty);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESIZED:
                    OnWindowSizeChanged?.Invoke((uint)e.window.windowID,
                        e.window.data1, e.window.data2);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    OnWindowClose?.Invoke((uint)e.window.windowID);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_GAINED:
                    OnWindowFocus?.Invoke((uint)e.window.windowID, true);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_FOCUS_LOST:
                    OnWindowFocus?.Invoke((uint)e.window.windowID, false);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_MINIMIZED:
                    OnWindowMinimized?.Invoke((uint)e.window.windowID, true);
                    break;

                case SDL.SDL_EventType.SDL_EVENT_WINDOW_RESTORED:
                    OnWindowMinimized?.Invoke((uint)e.window.windowID, false);
                    break;
            }
        }

        return !s_shouldQuit;
    }

    /// <summary>重置退出标志（用于重新进入主循环）。</summary>
    public static void ResetQuitFlag()
    {
        s_shouldQuit = false;
    }
}
