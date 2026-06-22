#pragma once

/**
 * @file EventTypes.h
 * @brief ABI-stable 事件类型定义：两级 NNEventType + NNEventSubtype + NNEvent。
 *
 * 架构：
 * - type = 粗分类（Engine-Level Category）：Window、Input、System、Scene…
 * - subtype = 细分类（Per-Type Specific）：Resized、ButtonDown、DropFile…
 * - SDL 是 Producer，不暴露到公共 ABI
 *
 * 约束：
 * - POD struct，trivially_copyable，standard_layout
 * - 128 字节固定大小，blittable，可 memcpy
 * - 无 vtable、无 std::string、无堆分配
 * - 枚举 append-only，永不重排或删除
 * - 字符串通过 Event String Pool 传递（见 EventAPI.h）
 */

#include <cstdint>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * NNEventType — 事件粗分类（Engine-Level Category）
 * Append-only. 4-63 预留给未来类型。
 * ======================================================================== */

typedef enum NNEventType
{
    NN_EVENT_TYPE_NONE    = 0,
    NN_EVENT_TYPE_WINDOW  = 1,   /* 窗口生命周期、焦点、显示 */
    NN_EVENT_TYPE_INPUT   = 2,   /* 鼠标、键盘、拖放 */
    NN_EVENT_TYPE_SYSTEM  = 3,   /* 应用退出 */
    /* 4-63 预留给未来：SCENE=4, ENTITY=5, ASSET=6, RENDERER=7, EDITOR=8... */
} NNEventType;

/* ========================================================================
 * NNWindowEventSubtype — 窗口事件细分类
 * Append-only. 22-255 预留。
 * ======================================================================== */

typedef enum NNWindowEventSubtype
{
    NN_WINDOW_NONE                =  0,
    NN_WINDOW_SHOWN               =  1,
    NN_WINDOW_HIDDEN              =  2,
    NN_WINDOW_EXPOSED             =  3,
    NN_WINDOW_MOVED               =  4,
    NN_WINDOW_RESIZED             =  5,
    NN_WINDOW_SIZE_CHANGED        =  6,
    NN_WINDOW_PIXEL_SIZE_CHANGED  =  7,
    NN_WINDOW_MINIMIZED           =  8,
    NN_WINDOW_MAXIMIZED           =  9,
    NN_WINDOW_RESTORED            = 10,
    NN_WINDOW_MOUSE_ENTER         = 11,
    NN_WINDOW_MOUSE_LEAVE         = 12,
    NN_WINDOW_FOCUS_GAINED        = 13,
    NN_WINDOW_FOCUS_LOST          = 14,
    NN_WINDOW_CLOSE               = 15,
    NN_WINDOW_DPI_CHANGED         = 16,
    NN_WINDOW_DISPLAY_CHANGED     = 17,
    NN_WINDOW_ENTER_FULLSCREEN    = 18,
    NN_WINDOW_LEAVE_FULLSCREEN    = 19,
    NN_WINDOW_TERMINATING         = 20,
    NN_WINDOW_LOW_MEMORY          = 21,
    /* 22-255 预留 */
} NNWindowEventSubtype;

/* ========================================================================
 * NNInputEventSubtype — 输入事件细分类（鼠标/键盘/拖放）
 * Append-only. 13-255 预留。
 * ======================================================================== */

typedef enum NNInputEventSubtype
{
    NN_INPUT_NONE              =  0,
    NN_INPUT_MOUSE_MOTION      =  1,
    NN_INPUT_MOUSE_BUTTON_DOWN =  2,
    NN_INPUT_MOUSE_BUTTON_UP   =  3,
    NN_INPUT_MOUSE_WHEEL       =  4,
    NN_INPUT_KEY_DOWN          =  5,
    NN_INPUT_KEY_UP            =  6,
    NN_INPUT_TEXT_INPUT        =  7,
    NN_INPUT_TEXT_EDITING      =  8,
    NN_INPUT_DROP_BEGIN        =  9,
    NN_INPUT_DROP_FILE         = 10,
    NN_INPUT_DROP_TEXT         = 11,
    NN_INPUT_DROP_COMPLETE     = 12,
    /* 13-255 预留 */
} NNInputEventSubtype;

/* ========================================================================
 * NNSystemEventSubtype — 系统事件细分类
 * Append-only. 2-255 预留。
 * ======================================================================== */

typedef enum NNSystemEventSubtype
{
    NN_SYSTEM_NONE = 0,
    NN_SYSTEM_QUIT = 1,
    /* 2-255 预留 */
} NNSystemEventSubtype;

/* ========================================================================
 * NNEvent — ABI 主事件结构体（128 字节）
 *
 * 内存布局：
 * ┌──────────────────────────────────────────────────────────────┐
 * │  offset 0x00  type          uint32     4B   粗分类            │
 * │  offset 0x04  subtype       uint32     4B   细分类            │
 * │  offset 0x08  timestamp     uint64     8B   时间戳 (ms)       │
 * │  offset 0x10  source        uint64     8B   来源句柄          │
 * │  offset 0x18  data1         int32      4B   通用数据 1        │
 * │  offset 0x1C  data2         int32      4B   通用数据 2        │
 * │  offset 0x20  data3         int32      4B   通用数据 3        │
 * │  offset 0x24  data4         int32      4B   通用数据 4        │
 * │  offset 0x28  flags         uint32     4B   事件标志          │
 * │  offset 0x2C  stringPoolIdx uint32     4B   字符串池索引      │
 * │  offset 0x30  reservedPad   [20]uint32 80B  预留（128B 对齐） │
 * └──────────────────────────────────────────────────────────────┘
 *
 * 字段语义（按 type+subtype 不同）：
 *
 * Window/Resized:              data1=width, data2=height
 * Window/Moved:                data1=x, data2=y
 * Window/DpiChanged:           data1=dpi×1000 (e.g. 144000=144.0)
 * Input/DropFile/DropText:     stringPoolIdx→路径, data1=x, data2=y
 * Input/MouseButtonDown:       data1=button, data2=x, data3=y, flags=clicks
 * Input/MouseMotion:           data1=x, data2=y, data3=xrel, data4=yrel
 * Input/MouseWheel:            data1=horz×1000, data2=vert×1000
 * Input/KeyDown/KeyUp:         data1=keycode, data2=scancode, data3=mods
 * Input/TextInput:             stringPoolIdx→UTF-8文本
 * System/Quit:                 (无附加数据)
 */
typedef struct NNEvent
{
    std::uint32_t type;             /* NNEventType              4B  0x00 */
    std::uint32_t subtype;          /* per-type subtype enum    4B  0x04 */
    std::uint64_t timestamp;        /* SDL ticks (ms)           8B  0x08 */
    std::uint64_t source;           /* NNWindowHandle (0=无)    8B  0x10 */

    std::int32_t  data1;            /* 通用整数数据 1           4B  0x18 */
    std::int32_t  data2;            /* 通用整数数据 2           4B  0x1C */
    std::int32_t  data3;            /* 通用整数数据 3           4B  0x20 */
    std::int32_t  data4;            /* 通用整数数据 4           4B  0x24 */

    std::uint32_t flags;            /* 事件特定标志             4B  0x28 */
    std::uint32_t stringPoolIdx;    /* String Pool 字节偏移     4B  0x2C */

    std::uint32_t reservedPad[20];  /* 80B 预留 → 总计 128 字节 */

} NNEvent;

/* 编译期验证 */
static_assert(sizeof(NNEvent) == 128,
    "NNEvent must be exactly 128 bytes");

#ifdef __cplusplus
}
#endif
