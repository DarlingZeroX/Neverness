#pragma once

/**
 * @file SDL3EventTranslator.h
 * @brief SDL_Event → NNEvent 翻译层（两级 type + subtype 架构）。
 *
 * 职责：
 * - 将 SDL3 原生事件转换为 ABI 稳定的 NNEvent（128 字节 POD）
 * - SDL 窗口句柄 → NNWindowHandle 映射（通过 WindowRegistry）
 * - DROP_FILE 路径写入 String Pool
 * - 不暴露任何 SDL 类型到公共 ABI
 *
 * 设计：
 * - type = 粗分类（Window/Input/System），subtype = 细分类
 * - SDL 是 Producer，事件翻译在此层完成
 * - 零堆分配（String Pool 内联）
 * - 不可翻译的事件返回 false（丢弃）
 * - 线程安全：仅由生产者（SDL 线程）调用
 */

#include "NNNativeEngineAPI/Include/EventTypes.h"
#include "NNNativeEngineAPI/Include/WindowTypes.h"
#include "WindowRegistry.h"
#include "EventQueue.h"

#include <SDL3/SDL.h>
#include <cstring>

namespace NN::Runtime::Application
{

/**
 * @brief SDL3 事件翻译器。
 * 持有 EventQueue 引用用于写入翻译后的事件和字符串。
 */
class SDL3EventTranslator
{
public:
    explicit SDL3EventTranslator(NN::Runtime::EventQueue& queue) noexcept
        : m_queue(queue) {}

	~SDL3EventTranslator() = default;
    /**
     * @brief 将 SDL_Event 翻译为 NNEvent 并推入队列。
     * @return true = 已入队，false = 丢弃（不可翻译或队列满）。
     */
    bool TranslateAndPush(const SDL_Event& sdl) noexcept
    {
        NNEvent ev{};
        ev.timestamp = sdl.common.timestamp;

        /* 识别来源句柄 */
        if (sdl.type >= SDL_EVENT_WINDOW_FIRST && sdl.type <= SDL_EVENT_WINDOW_LAST)
        {
            ev.source = ResolveWindowHandle(sdl.window.windowID);
        }
        else if (sdl.type >= SDL_EVENT_MOUSE_MOTION && sdl.type <= SDL_EVENT_MOUSE_BUTTON_UP)
        {
            ev.source = ResolveWindowHandle(sdl.motion.windowID);
        }
        else if (sdl.type >= SDL_EVENT_KEY_DOWN && sdl.type <= SDL_EVENT_KEY_UP)
        {
            ev.source = ResolveWindowHandle(sdl.key.windowID);
        }
        else if (sdl.type == SDL_EVENT_DROP_FILE || sdl.type == SDL_EVENT_DROP_TEXT ||
                 sdl.type == SDL_EVENT_DROP_BEGIN || sdl.type == SDL_EVENT_DROP_COMPLETE)
        {
            ev.source = ResolveWindowHandle(sdl.drop.windowID);
        }

        /* 翻译 */
        if (!TranslateWindowEvent(sdl, ev) &&
            !TranslateInputEvent(sdl, ev) &&
            !TranslateSystemEvent(sdl, ev))
        {
            return false; /* 不可翻译 */
        }

        return m_queue.PushOrCoalesceResize(ev);
    }

private:
    NN::Runtime::EventQueue& m_queue;

    /** @brief SDL_WindowID → NNWindowHandle 映射。 */
    NNWindowHandle ResolveWindowHandle(SDL_WindowID windowId) const noexcept
    {
        if (windowId == 0)
            return 0;

        SDL_Window* sdlWin = SDL_GetWindowFromID(windowId);
        if (sdlWin == nullptr)
            return 0;

        return WindowRegistry::FindHandle(sdlWin);
    }

    /* ── 窗口事件翻译 ── */

    bool TranslateWindowEvent(const SDL_Event& sdl, NNEvent& ev) noexcept
    {
        if (sdl.type < SDL_EVENT_WINDOW_FIRST || sdl.type > SDL_EVENT_WINDOW_LAST)
            return false;

        ev.type = NN_EVENT_TYPE_WINDOW;

        switch (sdl.type)
        {
        case SDL_EVENT_WINDOW_SHOWN:
            ev.subtype = NN_WINDOW_SHOWN;
            return true;

        case SDL_EVENT_WINDOW_HIDDEN:
            ev.subtype = NN_WINDOW_HIDDEN;
            return true;

        case SDL_EVENT_WINDOW_EXPOSED:
            ev.subtype = NN_WINDOW_EXPOSED;
            return true;

        case SDL_EVENT_WINDOW_MOVED:
            ev.subtype = NN_WINDOW_MOVED;
            ev.data1 = sdl.window.data1; /* x */
            ev.data2 = sdl.window.data2; /* y */
            return true;

        case SDL_EVENT_WINDOW_RESIZED:
            ev.subtype = NN_WINDOW_RESIZED;
            ev.data1 = sdl.window.data1; /* width */
            ev.data2 = sdl.window.data2; /* height */
            return true;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            ev.subtype = NN_WINDOW_PIXEL_SIZE_CHANGED;
            ev.data1 = sdl.window.data1;
            ev.data2 = sdl.window.data2;
            return true;

        case SDL_EVENT_WINDOW_MINIMIZED:
            ev.subtype = NN_WINDOW_MINIMIZED;
            return true;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            ev.subtype = NN_WINDOW_MAXIMIZED;
            return true;

        case SDL_EVENT_WINDOW_RESTORED:
            ev.subtype = NN_WINDOW_RESTORED;
            return true;

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            ev.subtype = NN_WINDOW_MOUSE_ENTER;
            return true;

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            ev.subtype = NN_WINDOW_MOUSE_LEAVE;
            return true;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            ev.subtype = NN_WINDOW_FOCUS_GAINED;
            return true;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            ev.subtype = NN_WINDOW_FOCUS_LOST;
            return true;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            ev.subtype = NN_WINDOW_CLOSE;
            return true;

        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            ev.subtype = NN_WINDOW_DPI_CHANGED;
            ev.data1 = sdl.window.data1;
            ev.data2 = sdl.window.data2;
            return true;

        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            ev.subtype = NN_WINDOW_ENTER_FULLSCREEN;
            return true;

        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            ev.subtype = NN_WINDOW_LEAVE_FULLSCREEN;
            return true;

        default:
            return false;
        }
    }

    /* ── 输入事件翻译（鼠标/键盘/拖放） ── */

    bool TranslateInputEvent(const SDL_Event& sdl, NNEvent& ev) noexcept
    {
        /* 鼠标 */
        if (sdl.type == SDL_EVENT_MOUSE_MOTION ||
            sdl.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
            sdl.type == SDL_EVENT_MOUSE_BUTTON_UP ||
            sdl.type == SDL_EVENT_MOUSE_WHEEL)
        {
            ev.type = NN_EVENT_TYPE_INPUT;

            switch (sdl.type)
            {
            case SDL_EVENT_MOUSE_MOTION:
                ev.subtype = NN_INPUT_MOUSE_MOTION;
                ev.data1 = static_cast<std::int32_t>(sdl.motion.x);
                ev.data2 = static_cast<std::int32_t>(sdl.motion.y);
                ev.data3 = static_cast<std::int32_t>(sdl.motion.xrel);
                ev.data4 = static_cast<std::int32_t>(sdl.motion.yrel);
                ev.flags = static_cast<std::uint32_t>(sdl.motion.state);
                return true;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                ev.subtype = NN_INPUT_MOUSE_BUTTON_DOWN;
                ev.data1 = sdl.button.button;
                ev.data2 = static_cast<std::int32_t>(sdl.button.x);
                ev.data3 = static_cast<std::int32_t>(sdl.button.y);
                ev.flags = sdl.button.clicks;
                return true;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                ev.subtype = NN_INPUT_MOUSE_BUTTON_UP;
                ev.data1 = sdl.button.button;
                ev.data2 = static_cast<std::int32_t>(sdl.button.x);
                ev.data3 = static_cast<std::int32_t>(sdl.button.y);
                ev.flags = sdl.button.clicks;
                return true;

            case SDL_EVENT_MOUSE_WHEEL:
                ev.subtype = NN_INPUT_MOUSE_WHEEL;
                ev.data1 = static_cast<std::int32_t>(sdl.wheel.x * 128);
                ev.data2 = static_cast<std::int32_t>(sdl.wheel.y * 128);
                ev.data3 = static_cast<std::int32_t>(sdl.wheel.mouse_x);
                ev.data4 = static_cast<std::int32_t>(sdl.wheel.mouse_y);
                return true;

            default:
                return false;
            }
        }

        /* 键盘 */
        if (sdl.type == SDL_EVENT_KEY_DOWN ||
            sdl.type == SDL_EVENT_KEY_UP ||
            sdl.type == SDL_EVENT_TEXT_INPUT ||
            sdl.type == SDL_EVENT_TEXT_EDITING)
        {
            ev.type = NN_EVENT_TYPE_INPUT;

            switch (sdl.type)
            {
            case SDL_EVENT_KEY_DOWN:
                ev.subtype = NN_INPUT_KEY_DOWN;
                ev.data1 = sdl.key.key;
                ev.data2 = sdl.key.scancode;
                ev.data3 = sdl.key.mod;
                ev.flags = sdl.key.repeat ? 1u : 0u;
                return true;

            case SDL_EVENT_KEY_UP:
                ev.subtype = NN_INPUT_KEY_UP;
                ev.data1 = sdl.key.key;
                ev.data2 = sdl.key.scancode;
                ev.data3 = sdl.key.mod;
                return true;

            case SDL_EVENT_TEXT_INPUT:
                ev.subtype = NN_INPUT_TEXT_INPUT;
                if (sdl.text.text != nullptr)
                {
                    const auto len = static_cast<std::uint16_t>(std::strlen(sdl.text.text));
                    if (len > 0)
                    {
                        ev.stringPoolIdx = m_queue.WriteString(sdl.text.text, len);
                        if (ev.stringPoolIdx == UINT32_MAX)
                            return false;
                    }
                }
                return true;

            case SDL_EVENT_TEXT_EDITING:
                ev.subtype = NN_INPUT_TEXT_EDITING;
                ev.data1 = sdl.edit.start;
                ev.data2 = sdl.edit.length;
                if (sdl.edit.text != nullptr)
                {
                    const auto len = static_cast<std::uint16_t>(std::strlen(sdl.edit.text));
                    if (len > 0)
                    {
                        ev.stringPoolIdx = m_queue.WriteString(sdl.edit.text, len);
                        if (ev.stringPoolIdx == UINT32_MAX)
                            return false;
                    }
                }
                return true;

            default:
                return false;
            }
        }

        /* 拖放 */
        if (sdl.type == SDL_EVENT_DROP_BEGIN ||
            sdl.type == SDL_EVENT_DROP_FILE ||
            sdl.type == SDL_EVENT_DROP_TEXT ||
            sdl.type == SDL_EVENT_DROP_COMPLETE)
        {
            ev.type = NN_EVENT_TYPE_INPUT;

            switch (sdl.type)
            {
            case SDL_EVENT_DROP_BEGIN:
                ev.subtype = NN_INPUT_DROP_BEGIN;
                return true;

            case SDL_EVENT_DROP_FILE:
                ev.subtype = NN_INPUT_DROP_FILE;
                if (sdl.drop.data != nullptr)
                {
                    const auto len = static_cast<std::uint16_t>(std::strlen(sdl.drop.data));
                    if (len > 0)
                    {
                        ev.stringPoolIdx = m_queue.WriteString(sdl.drop.data, len);
                        if (ev.stringPoolIdx == UINT32_MAX)
                            return false;
                    }
                }
                return true;

            case SDL_EVENT_DROP_TEXT:
                ev.subtype = NN_INPUT_DROP_TEXT;
                if (sdl.drop.data != nullptr)
                {
                    const auto len = static_cast<std::uint16_t>(std::strlen(sdl.drop.data));
                    if (len > 0)
                    {
                        ev.stringPoolIdx = m_queue.WriteString(sdl.drop.data, len);
                        if (ev.stringPoolIdx == UINT32_MAX)
                            return false;
                    }
                }
                return true;

            case SDL_EVENT_DROP_COMPLETE:
                ev.subtype = NN_INPUT_DROP_COMPLETE;
                return true;

            default:
                return false;
            }
        }

        return false;
    }

    /* ── 系统事件翻译 ── */

    bool TranslateSystemEvent(const SDL_Event& sdl, NNEvent& ev) noexcept
    {
        switch (sdl.type)
        {
        case SDL_EVENT_QUIT:
            ev.type = NN_EVENT_TYPE_SYSTEM;
            ev.subtype = NN_SYSTEM_QUIT;
            return true;

        case SDL_EVENT_TERMINATING:
            ev.type = NN_EVENT_TYPE_WINDOW;
            ev.subtype = NN_WINDOW_TERMINATING;
            return true;

        case SDL_EVENT_LOW_MEMORY:
            ev.type = NN_EVENT_TYPE_WINDOW;
            ev.subtype = NN_WINDOW_LOW_MEMORY;
            return true;

        default:
            return false;
        }
    }
};

} // namespace NN::Runtime::Application
