# Neverness Engine — Native Event Pump Architecture

> **Status**: Design Document
> **Author**: Claude + Neverness Engine Team
> **Date**: 2026-05-25
> **Scope**: Native Runtime → C# Event Bridge, Pull-Based ABI Event Queue

---

## 目录

1. [Current Architecture Analysis](#1-current-architecture-analysis)
2. [Problem Statement](#2-problem-statement)
3. [Architecture Decision: Pull-Based Event Queue](#3-architecture-decision-pull-based-event-queue)
4. [ABI Event Struct Design](#4-abi-event-struct-design)
5. [UTF-8 String Strategy for DROP_FILE](#5-utf-8-string-strategy-for-drop_file)
6. [Native Event Queue Design](#6-native-event-queue-design)
7. [SDL Translation Layer](#7-sdl-translation-layer)
8. [C ABI API Design](#8-c-abi-api-design)
9. [C# Interop Layer Design](#9-c-interop-layer-design)
10. [Editor Integration](#10-editor-integration)
11. [Threading Model](#11-threading-model)
12. [Multi-Window Strategy](#12-multi-window-strategy)
13. [ImGui Integration](#13-imgui-integration)
14. [Future-Proof Design](#14-future-proof-design)
15. [File Change Checklist](#15-file-change-checklist)
16. [Implementation Phases](#16-implementation-phases)

---

## 1. Current Architecture Analysis

### 1.1 Existing Event Types

**HEvent** (`NNCore/Include/Event/HEventBase.h`):
- 虚基类，`virtual GetEventType()`, `GetName()`, `GetCategoryFlags()`
- 内部 C++ 事件总线用，不可跨 ABI

**HWindowEvent** (`NNCore/Include/Event/HWindowEvents.h`):
- 继承 `HEvent`
- 包含 `std::string file` 字段（DROP_FILE 用）
- 通过 `HEventDelegate` + `eventpp::EventQueue` 在 C++ 内部广播
- **不可直接暴露到 ABI**（虚表 + std::string）

**现有事件类型**:
```
Window: SHOWN, HIDDEN, EXPOSED, MOVED, RESIZED, SIZE_CHANGED,
        MINIMIZED, MAXIMIZED, RESTORED, ENTER, LEAVE,
        FOCUS_GAINED, FOCUS_LOST, CLOSE, TAKE_FOCUS,
        ICCPROF_CHANGED, DISPLAY_CHANGED
Drop:   DROP_BEGIN, DROP_FILE, DROP_TEXT, DROP_COMPLETE
```

### 1.2 Existing ABI

**NNWindowAPI** — 窗口操作函数表（create/destroy/resize/move/show/hide）。
**NNApplicationAPI** — `pumpEvents` 返回 `bool`，不暴露事件细节。

**Gap**: 没有 C ABI 将 Window Events 传递到 C# 端。

### 1.3 Existing SDL Integration

`RuntimeApplication::PumpEvents()` → `SDL_PollEvent` → `VGWindow::ProcessEvent()`
→ `HWindowEvent` → `HEventDelegate` 广播。

事件只在 C++ 内部消费，C# 端完全不可见。

### 1.4 Existing C# Pattern

- `delegate* unmanaged` 函数指针（非 DllImport）
- `[StructLayout(LayoutKind.Sequential)]` blittable struct
- `EngineNativeApiBootstrap` 验证 `LayoutVersion`
- NativeAOT 兼容，零 GC Alloc

---

## 2. Problem Statement

### 2.1 当前问题

| 问题 | 影响 |
|------|------|
| C# 端无法获取 Window Events | Editor 无法响应 Drop、Resize、Focus |
| `HWindowEvent` 含 `std::string` | 不可跨 ABI |
| `HEvent` 有虚表 | 不可跨 ABI |
| `pumpEvents` 返回 bool | 事件丢失 |
| 无事件队列暴露 | C# 每帧无法主动拉取 |

### 2.2 设计约束

- **禁止**: SDL_Event / SDL_Window* 暴露到 ABI
- **禁止**: C# delegate callback（避免 GC pinning / callback 地狱）
- **禁止**: std::string / std::function 跨 ABI
- **禁止**: 每事件 P/Invoke 调用
- **必须**: POD struct，blittable，固定大小
- **必须**: Pull-based（C# 每帧主动拉取）
- **必须**: NativeAOT 兼容
- **必须**: ABI append-only，layoutVersion 控制

---

## 3. Architecture Decision: Pull-Based Event Queue

### 3.1 为什么 Pull-Based 而非 Callback

| 方案 | ABI 安全 | GC 安全 | 性能 | Debug | 可扩展 |
|------|----------|---------|------|-------|--------|
| **Callback (RegisterEventCallback)** | 需 pinning | delegate 泄漏 | 每事件 P/Invoke | 难追踪 | 差 |
| **Pull Queue (PollEvent)** | 纯 POD struct | 零 GC | 批量一次 | 易追踪 | 好 |

**决定**: Pull-Based Event Queue

- Native 端维护一个 lock-free ring buffer
- SDL 事件翻译为 POD `NNEvent` 写入队列
- C# 端每帧调用 `PollEvent(out NNEvent)` 批量消费
- 无 callback，无 pinning，无 GC alloc

### 3.2 对标

| 引擎 | 方案 |
|------|------|
| **Unreal** | `FPlatformEvents` → `FSlateApplication::PollGameDeviceState`（Pull） |
| **Unity** | `NativeEventType` → `Event.PopEvent`（Pull） |
| **Chromium** | `base::MessagePump` → `UIEvent` queue（Pull） |
| **Godot** | `DisplayServer::process_events` → `InputEvent` queue（Pull） |

---

## 4. ABI Event Struct Design

### 4.1 NNEventType — 事件类型枚举

```cpp
/// @brief ABI-stable event type enum.
/// Append-only. Never reorder or remove values.
/// New values added at end of each category block.
enum class NNEventType : std::uint32_t
{
    None = 0,

    // ─── Window Events [0x0001 - 0x00FF] ───
    WindowShown         = 0x0001,
    WindowHidden        = 0x0002,
    WindowExposed       = 0x0003,
    WindowMoved         = 0x0004,
    WindowResized       = 0x0005,
    WindowSizeChanged   = 0x0006,
    WindowMinimized     = 0x0007,
    WindowMaximized     = 0x0008,
    WindowRestored      = 0x0009,
    WindowMouseEnter    = 0x000A,
    WindowMouseLeave    = 0x000B,
    WindowFocusGained   = 0x000C,
    WindowFocusLost     = 0x000D,
    WindowClose         = 0x000E,
    WindowDpiChanged    = 0x000F,
    WindowDisplayChanged = 0x0010,
    // Reserved: 0x0011 - 0x00FF

    // ─── Drop Events [0x0100 - 0x01FF] ───
    DropBegin           = 0x0100,
    DropFile            = 0x0101,
    DropText            = 0x0102,
    DropComplete        = 0x0103,
    // Reserved: 0x0104 - 0x01FF

    // ─── Mouse Events [0x0200 - 0x02FF] ─── (Future)
    MouseButtonDown     = 0x0200,
    MouseButtonUp       = 0x0201,
    MouseMotion         = 0x0202,
    MouseWheel          = 0x0203,
    // Reserved: 0x0204 - 0x02FF

    // ─── Keyboard Events [0x0300 - 0x03FF] ─── (Future)
    KeyDown             = 0x0300,
    KeyUp               = 0x0301,
    KeyText             = 0x0302,   ///< Text input (UTF-8 via string pool)
    // Reserved: 0x0303 - 0x03FF

    // ─── System Events [0x0400 - 0x04FF] ───
    Quit                = 0x0400,
    // Reserved: 0x0401 - 0x04FF

    // ─── User Events [0x8000 - 0xFFFF] ───
    UserEvent           = 0x8000,
    // 0x8001 - 0xFFFF available for engine-internal user events
};
```

### 4.2 NNEventCategory — 事件分类位掩码

```cpp
/// @brief Event category bitmask for fast filtering.
enum class NNEventCategory : std::uint32_t
{
    None        = 0,
    Window      = 1u << 0,
    Drop        = 1u << 1,
    Mouse       = 1u << 2,
    Keyboard    = 1u << 3,
    System      = 1u << 4,
    User        = 1u << 5,
    // Reserved: bit 6-31
};

inline NNEventCategory operator|(NNEventCategory a, NNEventCategory b) noexcept
{
    return static_cast<NNEventCategory>(
        static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}
```

### 4.3 NNEvent — 主事件结构体（核心 ABI）

```cpp
/// @brief ABI-stable event struct.
///
/// Fixed 128 bytes. Blittable. Trivially copyable.
/// No vtable, no std::string, no heap allocation.
///
/// Layout:
/// ┌──────────────────────────────────────────────────────────────┐
/// │  offset 0x00  type          uint32     4B   事件类型          │
/// │  offset 0x04  category      uint32     4B   事件分类          │
/// │  offset 0x08  timestamp     uint64     8B   SDL ticks (ms)    │
/// │  offset 0x10  windowHandle  uint64     8B   窗口句柄          │
/// │  offset 0x18  data1         int32      4B   通用数据 1        │
/// │  offset 0x1C  data2         int32      4B   通用数据 2        │
/// │  offset 0x20  data3         int32      4B   通用数据 3        │
/// │  offset 0x24  data4         int32      4B   通用数据 4        │
/// │  offset 0x28  flags         uint32     4B   事件标志          │
/// │  offset 0x2C  reserved0     uint32     4B   对齐 + 预留       │
/// │  offset 0x30  stringPoolIdx uint32     4B   字符串池索引      │
/// │  offset 0x34  reservedPad   [12]uint32 48B  预留（128B 对齐） │
/// └──────────────────────────────────────────────────────────────┘
///
/// 128 字节 = cache line × 2，GPU/SIMD 友好。
///
/// 字段语义（根据 type 不同）:
///
/// WindowResized / WindowSizeChanged:
///   data1 = width, data2 = height
///
/// WindowMoved:
///   data1 = x, data2 = y
///
/// WindowDpiChanged:
///   data1 = dpi (fixed-point × 1000, e.g. 144000 = 144.0)
///
/// DropFile / DropText:
///   stringPoolIdx → 字符串池中的 UTF-8 路径
///   data1 = x, data2 = y (drop 坐标)
///
/// MouseButtonDown / MouseButtonUp:
///   data1 = button (1=left, 2=middle, 3=right)
///   data2 = x, data3 = y
///   data4 = clicks (1=single, 2=double)
///
/// MouseMotion:
///   data1 = x, data2 = y
///   data3 = xrel, data4 = yrel
///
/// MouseWheel:
///   data1 = x (horizontal), data2 = y (vertical, float→int × 1000)
///   data3 = mouseX, data4 = mouseY
///
/// KeyDown / KeyUp:
///   data1 = scancode, data2 = keycode, data3 = mod state
///
/// KeyText:
///   stringPoolIdx → 字符串池中的 UTF-8 文本
///
/// UserEvent:
///   data1-4 = user-defined
///   flags = user subtype
struct NNEvent
{
    std::uint32_t type          = 0;   // NNEventType
    std::uint32_t category      = 0;   // NNEventCategory
    std::uint64_t timestamp     = 0;   // SDL_GetTicksNS or similar
    std::uint64_t windowHandle  = 0;   // NNWindowHandle

    std::int32_t  data1         = 0;
    std::int32_t  data2         = 0;
    std::int32_t  data3         = 0;
    std::int32_t  data4         = 0;

    std::uint32_t flags         = 0;
    std::uint32_t reserved0     = 0;
    std::uint32_t stringPoolIdx = 0;   // Index into event string pool

    std::uint32_t reservedPad[12] = {}; // 12 × 4 = 48B padding → 128B total
};

static_assert(sizeof(NNEvent) == 128, "NNEvent must be exactly 128 bytes");
static_assert(std::is_trivially_copyable_v<NNEvent>, "NNEvent must be trivially copyable");
static_assert(std::is_standard_layout_v<NNEvent>, "NNEvent must have standard layout");
```

### 4.4 为什么 128 字节

| 理由 | 说明 |
|------|------|
| Cache line 对齐 | 128 = 2 × 64B，避免 false sharing |
| 预留空间 | Input 事件需要更多字段（scancode, keycode, mods, etc.） |
| 固定 stride | Ring buffer 可直接 index，无需解析 |
| SIMD 友好 | 对齐到 16B 边界 |
| 未来扩展 | 新字段写入 reservedPad，不破坏 ABI |

---

## 5. UTF-8 String Strategy for DROP_FILE

### 5.1 方案分析

| 方案 | ABI 稳定 | GC 安全 | NativeAOT | 性能 | 大路径 | 复杂度 |
|------|----------|---------|-----------|------|--------|--------|
| **char[260] in struct** | 好 | 好 | 好 | 最快 | ❌ Windows MAX_PATH=260, 但 Linux 无限制 | 低 |
| **char[4096] in struct** | 好 | 好 | 好 | 快 | ⚠️ 大部分够用，但每事件浪费 4KB | 低 |
| **Event String Pool (Separate Buffer)** | 好 | 好 | 好 | 快 | ✅ 无限制 | 中 |
| **Separate Blob Buffer** | 好 | 好 | 好 | 快 | ✅ 无限制 | 中 |
| **Interned String Handle** | 好 | 好 | 好 | 最快 | ✅ 无限制 | 高 |
| **malloc + pointer in struct** | ❌ ownership 混乱 | ❌ 需 free | ❌ 需 free | - | ✅ | - |

### 5.2 决定：Event String Pool（推荐）

**设计**:

```
┌──────────────────────────────────────────────────────┐
│  NNEventQueue                                        │
│  ├── Ring Buffer: NNEvent[4096]    (固定 512KB)      │
│  └── String Pool:  uint8[65536]    (固定 64KB)       │
│      └── format: [uint16 len][UTF-8 bytes]...        │
└──────────────────────────────────────────────────────┘
```

- `NNEvent.stringPoolIdx` 是 String Pool 中的 **字节偏移量**
- 每条字符串格式: `[uint16 length][...UTF-8 bytes...]`
- String Pool 也是一个 ring buffer，旧字符串被新字符串覆盖
- C# 端通过 `NNGetString(event, out byte* ptr, out int len)` 读取

**优势**:

| 属性 | 说明 |
|------|------|
| ABI 稳定 | NNEvent 仍然是 128B 固定，无指针 |
| GC 安全 | 字符串池在 Native 堆上，C# 只读 Span<byte> |
| NativeAOT | 纯 blittable，无需 marshalling |
| 性能 | 一次 memcpy 到 Span，零 alloc |
| 大路径 | 64KB pool 可存 ~200 条 300 字节路径 |
| 复杂度 | 中等——ring buffer + offset 管理 |

**String Pool 环形覆盖策略**:

```
写入时：
  if (writePos + 2 + utf8Len > poolSize)
      writePos = 0;  // 回卷（覆盖旧数据）
  pool[writePos] = (uint16)utf8Len;
  memcpy(pool + writePos + 2, utf8Bytes, utf8Len);
  stringPoolIdx = writePos;
  writePos += 2 + utf8Len;

读取时：
  len = *(uint16*)(pool + event.stringPoolIdx);
  ptr = pool + event.stringPoolIdx + 2;
```

**安全性保证**:

- String Pool 是 **单生产者（SDL 线程）写，单消费者（主线程 C#）读**
- PollEvent 时：先读 event，再读 string → 原子性由 SPSC ring buffer 保证
- 如果 C# 消费太慢导致 pool 被覆盖 → 路径字符串损坏（极端情况）
  - 缓解：Pool 大小 64KB，正常帧率下不可能覆盖
  - 缓解：C# 每帧消费所有事件后再做其他处理

### 5.3 替代方案：固定缓冲区（简单场景）

对于不需要大路径的场景，可以在 NNEvent 内嵌一个小缓冲区：

```cpp
// 不推荐，仅作为备选
struct NNEventWithBuffer
{
    NNEvent header;                          // 128B
    char    textBuffer[128];                 // 128B → 总 256B
};
// 问题：256B per event × 4096 = 1MB ring buffer，浪费
// 问题：128B 限制了长路径
```

**不采用**。Event String Pool 方案更优。

---

## 6. Native Event Queue Design

### 6.1 架构概览

```
SDL Thread (主线程)               C# Consumer (同线程)
┌──────────────────┐              ┌──────────────────┐
│ SDL_PollEvent    │              │ while(PollEvent) │
│      ↓           │              │      ↑           │
│ TranslateSDL()   │              │                  │
│      ↓           │              │                  │
│ ┌──────────────┐ │              │ ┌──────────────┐ │
│ │ Ring Buffer  │◄├── write ────┤►│  read index   │ │
│ │ NNEvent[4096]│ │              │ │              │ │
│ └──────────────┘ │              │ └──────────────┘ │
│ ┌──────────────┐ │              │ ┌──────────────┐ │
│ │ String Pool  │◄├── write ────┤►│  read string  │ │
│ │ uint8[65536] │ │              │ │              │ │
│ └──────────────┘ │              │ └──────────────┘ │
└──────────────────┘              └──────────────────┘
        Single Producer                   Single Consumer
```

### 6.2 SPSC Ring Buffer

**为什么 SPSC（Single Producer Single Consumer）**:

- SDL 事件只在主线程 `SDL_PollEvent`（SDL 要求）
- C# 消费也只在主线程（Editor/Game 主循环）
- SPSC 无需锁，原子操作即可
- 性能最高

**实现**:

```cpp
/// @brief Lock-free SPSC event ring buffer.
class NNEventQueue
{
public:
    static constexpr std::uint32_t kCapacity  = 4096;  // 必须是 2 的幂
    static constexpr std::uint32_t kMask      = kCapacity - 1;
    static constexpr std::uint32_t kPoolSize  = 65536;  // String pool 64KB

    /// @brief 生产者写入事件（SDL 线程调用）。
    bool Push(const NNEvent& event) noexcept
    {
        const auto write = m_write.load(std::memory_order_relaxed);
        const auto nextWrite = (write + 1) & kMask;

        // 检查是否满（保留一个 slot 作为空/满区分）
        if (nextWrite == m_read.load(std::memory_order_acquire))
            return false;  // 队列满，丢弃事件

        m_buffer[write] = event;
        m_write.store(nextWrite, std::memory_order_release);
        return true;
    }

    /// @brief 消费者读取事件（主线程 C# 调用）。
    bool Pop(NNEvent& out) noexcept
    {
        const auto read = m_read.load(std::memory_order_relaxed);

        if (read == m_write.load(std::memory_order_acquire))
            return false;  // 队列空

        out = m_buffer[read];
        m_read.store((read + 1) & kMask, std::memory_order_release);
        return true;
    }

    /// @brief 写入字符串到 String Pool（生产者调用）。
    /// @return String pool index, or UINT32_MAX if pool full.
    std::uint32_t WriteString(const char* utf8, std::uint16_t len) noexcept
    {
        const std::uint32_t totalLen = 2 + len;

        // 简单线性分配 + 回卷（SPSC 下安全）
        auto pos = m_poolWrite.load(std::memory_order_relaxed);

        if (pos + totalLen > kPoolSize)
        {
            // 回卷到头部
            pos = 0;
        }

        // 检查是否覆盖消费者读取位置
        auto poolRead = m_poolRead.load(std::memory_order_acquire);
        if (pos < poolRead && pos + totalLen >= poolRead)
        {
            return UINT32_MAX;  // Pool 满
        }

        // 写入长度 + 数据
        auto* dst = m_stringPool + pos;
        std::memcpy(dst, &len, 2);
        std::memcpy(dst + 2, utf8, len);

        std::uint32_t result = pos;
        m_poolWrite.store(pos + totalLen, std::memory_order_release);
        return result;
    }

    /// @brief 从 String Pool 读取字符串（消费者调用）。
    bool ReadString(std::uint32_t idx, const char*& outPtr,
                    std::uint16_t& outLen) const noexcept
    {
        if (idx >= kPoolSize) return false;
        std::memcpy(&outLen, m_stringPool + idx, 2);
        outPtr = reinterpret_cast<const char*>(m_stringPool + idx + 2);
        return true;
    }

private:
    alignas(64) NNEvent m_buffer[kCapacity]{};  // 对齐到 cache line

    alignas(64) std::atomic<std::uint32_t> m_write{0};  // 生产者游标
    alignas(64) std::atomic<std::uint32_t> m_read{0};   // 消费者游标

    alignas(64) std::uint8_t m_stringPool[kPoolSize]{};  // String Pool

    alignas(64) std::atomic<std::uint32_t> m_poolWrite{0};
    alignas(64) std::atomic<std::uint32_t> m_poolRead{0};
};

static_assert(sizeof(NNEventQueue) % 64 == 0,
    "NNEventQueue should be cache-line aligned");
```

### 6.3 队列满处理策略

| 场景 | 策略 |
|------|------|
| Event Queue 满 | 丢弃新事件，log warning |
| String Pool 满 | 丢弃事件（不写入不含路径的半成品事件） |
| Resize spam | 不丢弃——合并同一帧内的连续 RESIZE 事件 |
| 事件洪泛 | 单帧消费上限（max 256 events/frame） |

### 6.4 Resize Event 合并

连续的 `WindowResized` 事件可以合并：

```cpp
// 在 Push() 之前检查
bool TryCoalesceResize(const NNEvent& newEvent) noexcept
{
    if (newEvent.type != NNEventType::WindowResized)
        return false;

    // 检查队列尾部是否已有同一窗口的 resize
    auto write = m_write.load(std::memory_order_relaxed);
    auto prevIdx = (write - 1) & m_mask;
    auto& prev = m_buffer[prevIdx];

    if (prev.type == NNEventType::WindowResized &&
        prev.windowHandle == newEvent.windowHandle)
    {
        // 覆盖旧的 resize（保留最新的尺寸）
        prev = newEvent;
        return true;  // 已合并
    }

    return false;  // 不合并，正常入队
}
```

### 6.5 队列所有权

```
NNEventQueue 实例生命周期：

  RuntimeApplication::Initialize()
      ↓
  m_eventQueue = new NNEventQueue()     // Native 堆分配
      ↓
  NNEventAPI 持有指针
      ↓
  C# 通过 ABI 函数访问
      ↓
  RuntimeApplication::Shutdown()
      ↓
  delete m_eventQueue
```

**不要用全局静态** — 避免 DLL 边界问题。通过 API 函数表传递实例指针。

---

## 7. SDL Translation Layer

### 7.1 三层架构

```
Layer 0: SDL Platform (C, 外部依赖)
  │
  │  SDL_Event (SDL 内部结构，不可跨 ABI)
  │
  ▼
Layer 1: SDL Translation (Runtime Internal)
  │  文件: SDL3EventTranslator.h/.cpp
  │  所属: NNRuntimeApplication (不对外暴露)
  │  职责: SDL_Event → NNEvent + string pool 写入
  │
  ▼
Layer 2: NNEvent (ABI POD, 公开)
  │  文件: NNEventAPI.h
  │  所属: NNNativeEngineAPI (对外 ABI)
  │  职责: C# 通过 NNEventAPI 消费
  │
  ▼
Layer 3: C# Event Pump
     文件: EventApi.cs
     所属: Neverness.Runtime.Engine
     职责: while(PollEvent) 循环 + 事件路由
```

### 7.2 SDL3EventTranslator

```cpp
/// @brief SDL Event → NNEvent 翻译器。
/// Runtime 内部类，不暴露到 ABI。
class SDL3EventTranslator
{
public:
    /// @brief 将 SDL_Event 翻译为 NNEvent 并推入队列。
    /// @return true=成功入队，false=队列满或事件被过滤。
    bool TranslateAndPush(
        NNEventQueue& queue,
        const SDL_Event& sdl,
        NNWindowHandle activeWindow) noexcept
    {
        NNEvent evt{};
        evt.timestamp = SDL_GetTicks();

        switch (sdl.type)
        {
        // ─── Window Events ───────────────────────────
        case SDL_EVENT_WINDOW_SHOWN:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowShown);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_HIDDEN:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowHidden);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_MOVED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowMoved);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            evt.data1 = sdl.window.data1;  // x
            evt.data2 = sdl.window.data2;  // y
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_RESIZED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowResized);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            evt.data1 = sdl.window.data1;  // width
            evt.data2 = sdl.window.data2;  // height
            if (queue.TryCoalesceResize(evt)) return true;
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowSizeChanged);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            evt.data1 = sdl.window.data1;  // width
            evt.data2 = sdl.window.data2;  // height
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_MINIMIZED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowMinimized);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_MAXIMIZED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowMaximized);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_RESTORED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowRestored);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowMouseEnter);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowMouseLeave);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowFocusGained);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowFocusLost);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowClose);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            return queue.Push(evt);

        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            evt.type     = static_cast<std::uint32_t>(NNEventType::WindowDisplayChanged);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Window);
            evt.windowHandle = LookupWindowHandle(sdl.window.windowID);
            evt.data1 = sdl.window.data1;  // new display ID
            return queue.Push(evt);

        // ─── Drop Events ────────────────────────────
        case SDL_EVENT_DROP_BEGIN:
            evt.type     = static_cast<std::uint32_t>(NNEventType::DropBegin);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Drop);
            evt.windowHandle = LookupWindowHandle(sdl.drop.windowID);
            return queue.Push(evt);

        case SDL_EVENT_DROP_FILE:
        {
            evt.type     = static_cast<std::uint32_t>(NNEventType::DropFile);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Drop);
            evt.windowHandle = LookupWindowHandle(sdl.drop.windowID);
            evt.data1 = static_cast<std::int32_t>(sdl.drop.x);
            evt.data2 = static_cast<std::int32_t>(sdl.drop.y);

            // 写入路径到 String Pool
            if (sdl.drop.data != nullptr)
            {
                auto pathLen = static_cast<std::uint16_t>(
                    std::strlen(sdl.drop.data));
                auto poolIdx = queue.WriteString(sdl.drop.data, pathLen);
                if (poolIdx == UINT32_MAX) return false;  // Pool 满
                evt.stringPoolIdx = poolIdx;
            }
            return queue.Push(evt);
        }

        case SDL_EVENT_DROP_TEXT:
        {
            evt.type     = static_cast<std::uint32_t>(NNEventType::DropText);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Drop);
            evt.windowHandle = LookupWindowHandle(sdl.drop.windowID);
            evt.data1 = static_cast<std::int32_t>(sdl.drop.x);
            evt.data2 = static_cast<std::int32_t>(sdl.drop.y);

            if (sdl.drop.data != nullptr)
            {
                auto textLen = static_cast<std::uint16_t>(
                    std::strlen(sdl.drop.data));
                auto poolIdx = queue.WriteString(sdl.drop.data, textLen);
                if (poolIdx == UINT32_MAX) return false;
                evt.stringPoolIdx = poolIdx;
            }
            return queue.Push(evt);
        }

        case SDL_EVENT_DROP_COMPLETE:
            evt.type     = static_cast<std::uint32_t>(NNEventType::DropComplete);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::Drop);
            evt.windowHandle = LookupWindowHandle(sdl.drop.windowID);
            return queue.Push(evt);

        // ─── Quit ────────────────────────────────────
        case SDL_EVENT_QUIT:
            evt.type     = static_cast<std::uint32_t>(NNEventType::Quit);
            evt.category = static_cast<std::uint32_t>(NNEventCategory::System);
            return queue.Push(evt);

        // ─── Future: Input ───────────────────────────
        // case SDL_EVENT_MOUSE_BUTTON_DOWN: ...
        // case SDL_EVENT_KEY_DOWN: ...

        default:
            return false;  // 未知事件，忽略
        }
    }

private:
    /// @brief SDL_WindowID → NNWindowHandle 映射。
    /// 由 WindowRegistry 提供。
    NNWindowHandle LookupWindowHandle(std::uint32_t sdlWindowId) const noexcept;
};
```

### 7.3 SDL 资源生命周期

```
SDL_Event 内存归 SDL 所有（栈上）。
  ↓
SDL3EventTranslator::TranslateAndPush()
  ↓
  - DropFile: SDL 提供的 sdl.drop.data 是 SDL 内部 malloc
    → 立即 strlen + WriteString 到我们的 String Pool
    → 不持有 SDL 指针，不调用 SDL_free
    → SDL 在下一个 SDL_PollEvent 前会释放它
  ↓
  - 其他事件: 纯值拷贝到 NNEvent
  ↓
返回。SDL 内存由 SDL 自行管理。
```

**关键**: 我们 **不持有** SDL_Event 的任何指针。在 `TranslateAndPush` 返回前，所有需要的数据已拷贝到 `NNEvent` + String Pool。

---

## 8. C ABI API Design

### 8.1 NNEventAPI — 事件函数表

```c
/// @brief Event API function table.
/// Added to NNNativeEngineAPI as 'events' field.
/// ABI append-only. Layout version bump on addition.
typedef struct NNEventAPI
{
    std::uint32_t size;  // sizeof(NNEventAPI) for forward compat

    /// @brief Poll one event from the queue.
    /// @param outEvent [out] Receives the event data.
    /// @return 1 = event dequeued, 0 = queue empty.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * pollEvent)(
        NNEvent* outEvent);

    /// @brief Peek at next event without consuming.
    /// @param outEvent [out] Receives a copy of the next event.
    /// @return 1 = event available, 0 = queue empty.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * peekEvent)(
        NNEvent* outEvent);

    /// @brief Wait (block) for next event with timeout.
    /// @param outEvent [out] Receives the event.
    /// @param timeoutMs Maximum wait time in milliseconds (0 = poll).
    /// @return 1 = event dequeued, 0 = timeout.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * waitEvent)(
        NNEvent* outEvent,
        std::uint32_t timeoutMs);

    /// @brief Read string from event's string pool reference.
    /// @param event [in] The event containing stringPoolIdx.
    /// @param outPtr [out] Pointer to UTF-8 string (valid until next poll).
    /// @param outLen [out] Length in bytes.
    /// @return 1 = string found, 0 = no string or invalid index.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * getEventString)(
        const NNEvent* event,
        const char** outPtr,
        std::uint16_t* outLen);

    /// @brief Get the total number of events currently in queue.
    /// Useful for batch allocation on C# side.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * getQueueCount)(void);

    /// @brief Flush all events from queue (e.g., on window focus loss).
    void (NN_ENGINE_ABI_STDCALL * flushEvents)(void);

    /// @brief Push a user-defined event (for C# → Native communication).
    /// @return 1 = success, 0 = queue full.
    std::uint32_t (NN_ENGINE_ABI_STDCALL * pushUserEvent)(
        const NNEvent* event);

} NNEventAPI;
```

### 8.2 集成到 NNNativeEngineAPI

修改 `EngineAPIRegistry.h`，在 `NNNativeEngineAPI` 中添加 `events` 字段：

```c
typedef struct NNNativeEngineAPI {
    std::uint32_t layoutVersion;  // 18 → 19 (bump)
    // ... existing fields: render, ui, audio, ...
    const struct NNWindowAPI*       window;
    const struct NNEventAPI*        events;        // ← NEW
    // ... vfs, assetManager, assetCooker
} NNNativeEngineAPI;
```

**规则**: 新字段追加到末尾。旧 C# 代码通过 `LayoutVersion` 检测：
- Version 18: 无 `events` 字段
- Version 19+: 有 `events` 字段

### 8.3 实现构建

```cpp
// BuildEventApi.cpp (新文件，NNRuntimeApplication)
void NNBuildEventRuntimeApi(NNEventAPI* outApi, NNEventQueue* queue)
{
    outApi->size = sizeof(NNEventAPI);

    outApi->pollEvent = [](NNEvent* out) -> std::uint32_t {
        auto* q = GetGlobalEventQueue();  // TLS 或全局
        return q->Pop(*out) ? 1 : 0;
    };

    outApi->peekEvent = [](NNEvent* out) -> std::uint32_t {
        auto* q = GetGlobalEventQueue();
        return q->Peek(*out) ? 1 : 0;
    };

    outApi->waitEvent = [](NNEvent* out, std::uint32_t ms) -> std::uint32_t {
        auto* q = GetGlobalEventQueue();
        if (q->Pop(*out)) return 1;
        if (ms == 0) return 0;
        // Spin + sleep for simplicity (or use condition variable)
        auto deadline = SDL_GetTicks() + ms;
        while (SDL_GetTicks() < deadline)
        {
            if (q->Pop(*out)) return 1;
            SDL_Delay(1);
        }
        return 0;
    };

    outApi->getEventString = [](const NNEvent* e, const char** ptr,
                                 std::uint16_t* len) -> std::uint32_t {
        auto* q = GetGlobalEventQueue();
        return q->ReadString(e->stringPoolIdx, *ptr, *len) ? 1 : 0;
    };

    outApi->getQueueCount = []() -> std::uint32_t {
        return GetGlobalEventQueue()->Size();
    };

    outApi->flushEvents = []() {
        GetGlobalEventQueue()->Clear();
    };

    outApi->pushUserEvent = [](const NNEvent* e) -> std::uint32_t {
        return GetGlobalEventQueue()->Push(*e) ? 1 : 0;
    };
}
```

---

## 9. C# Interop Layer Design

### 9.1 NNEvent — C# 镜像结构体

```csharp
/// <summary>
/// ABI-stable event struct — 与 Native NNEvent 内存布局严格对齐（128 字节）。
/// Blittable，NativeAOT 兼容，零 GC Alloc。
/// </summary>
[StructLayout(LayoutKind.Sequential, Size = 128)]
public struct NNEvent
{
    public uint Type;           // 4B  offset 0
    public uint Category;       // 4B  offset 4
    public ulong Timestamp;     // 8B  offset 8
    public ulong WindowHandle;  // 8B  offset 16

    public int Data1;           // 4B  offset 24
    public int Data2;           // 4B  offset 28
    public int Data3;           // 4B  offset 32
    public int Data4;           // 4B  offset 36

    public uint Flags;          // 4B  offset 40
    public uint Reserved0;      // 4B  offset 44
    public uint StringPoolIdx;  // 4B  offset 48

    // 12 × 4 = 48B reserved padding (offset 52 - 99)
    // Total = 128B
}

static_assert: Marshal.SizeOf<NNEvent>() == 128
```

### 9.2 NNEventType — C# 枚举

```csharp
/// <summary>
/// Event type enum — 与 Native NNEventType 严格对应。
/// </summary>
public enum NNEventType : uint
{
    None = 0,

    // Window [0x0001 - 0x00FF]
    WindowShown          = 0x0001,
    WindowHidden         = 0x0002,
    WindowExposed        = 0x0003,
    WindowMoved          = 0x0004,
    WindowResized        = 0x0005,
    WindowSizeChanged    = 0x0006,
    WindowMinimized      = 0x0007,
    WindowMaximized      = 0x0008,
    WindowRestored       = 0x0009,
    WindowMouseEnter     = 0x000A,
    WindowMouseLeave     = 0x000B,
    WindowFocusGained    = 0x000C,
    WindowFocusLost      = 0x000D,
    WindowClose          = 0x000E,
    WindowDpiChanged     = 0x000F,
    WindowDisplayChanged = 0x0010,

    // Drop [0x0100 - 0x01FF]
    DropBegin            = 0x0100,
    DropFile             = 0x0101,
    DropText             = 0x0102,
    DropComplete         = 0x0103,

    // Mouse [0x0200 - 0x02FF] (Future)
    MouseButtonDown      = 0x0200,
    MouseButtonUp        = 0x0201,
    MouseMotion          = 0x0202,
    MouseWheel           = 0x0203,

    // Keyboard [0x0300 - 0x03FF] (Future)
    KeyDown              = 0x0300,
    KeyUp                = 0x0301,
    KeyText              = 0x0302,

    // System [0x0400 - 0x04FF]
    Quit                 = 0x0400,

    // User [0x8000 - 0xFFFF]
    UserEvent            = 0x8000,
}
```

### 9.3 NNEventApi — C# 函数表

```csharp
/// <summary>
/// Event API function table — 与 Native NNEventAPI 内存布局对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNEventApi
{
    public uint Size;

    public delegate* unmanaged<NNEvent*, uint> PollEvent;
    public delegate* unmanaged<NNEvent*, uint> PeekEvent;
    public delegate* unmanaged<NNEvent*, uint, uint> WaitEvent;
    public delegate* unmanaged<NNEvent*, byte**, ushort*, uint> GetEventString;
    public delegate* unmanaged<uint> GetQueueCount;
    public delegate* unmanaged<void> FlushEvents;
    public delegate* unmanaged<NNEvent*, uint> PushUserEvent;
}
```

### 9.4 EventApi — C# 高层封装

```csharp
/// <summary>
/// 事件 API 高层封装——提供类型安全的事件轮询接口。
/// 每帧调用 EventApi.PollEvents() 消费所有待处理事件。
/// </summary>
public static unsafe class EventApi
{
    private static NNEventApi* s_api;

    internal static void Install(NNEventApi* api)
    {
        s_api = api;
    }

    /// <summary>
    /// 消费一个事件。
    /// </summary>
    public static bool PollEvent(out NNEvent evt)
    {
        fixed (NNEvent* p = &evt)
        {
            return s_api->PollEvent(p) != 0;
        }
    }

    /// <summary>
    /// 批量消费所有事件到 Span（零 alloc）。
    /// 返回实际读取的数量。
    /// </summary>
    public static int PollEvents(Span<NNEvent> buffer)
    {
        int count = 0;
        fixed (NNEvent* p = buffer)
        {
            while (count < buffer.Length)
            {
                if (s_api->PollEvent(p + count) == 0)
                    break;
                count++;
            }
        }
        return count;
    }

    /// <summary>
    /// 从事件的 String Pool 引用中读取 UTF-8 字符串。
    /// 返回的 Span 在下次 PollEvent 前有效。
    /// </summary>
    public static bool GetEventString(in NNEvent evt, out ReadOnlySpan<byte> str)
    {
        fixed (NNEvent* pEvt = &evt)
        {
            byte* ptr;
            ushort len;
            if (s_api->GetEventString(pEvt, &ptr, &len) != 0)
            {
                str = new ReadOnlySpan<byte>(ptr, len);
                return true;
            }
        }
        str = default;
        return false;
    }

    /// <summary>
    /// 从 DropFile 事件获取 UTF-8 文件路径。
    /// </summary>
    public static bool GetDropFilePath(in NNEvent evt, out string path)
    {
        if (evt.Type == (uint)NNEventType.DropFile &&
            GetEventString(in evt, out var utf8))
        {
            path = Encoding.UTF8.GetString(utf8);
            return true;
        }
        path = string.Empty;
        return false;
    }

    /// <summary>
    /// 窗口 Resize 事件获取新尺寸。
    /// </summary>
    public static (int width, int height) GetWindowSize(in NNEvent evt)
    {
        return (evt.Data1, evt.Data2);
    }

    /// <summary>
    /// 窗口 Move 事件获取新位置。
    /// </summary>
    public static (int x, int y) GetWindowPosition(in NNEvent evt)
    {
        return (evt.Data1, evt.Data2);
    }

    /// <summary>
    /// 获取队列中事件数量。
    /// </summary>
    public static int QueueCount => (int)s_api->GetQueueCount();

    /// <summary>
    /// 清空队列。
    /// </summary>
    public static void Flush() => s_api->FlushEvents();

    /// <summary>
    /// 推送用户事件（C# → Native）。
    /// </summary>
    public static bool PushUserEvent(NNEvent evt)
    {
        return s_api->PushUserEvent(&evt) != 0;
    }
}
```

### 9.5 Bootstrap 集成

修改 `EngineNativeApiBootstrap.cs`：

```csharp
// 在 InstallFromNativeApiTable 中：
if (nativeApi.LayoutVersion >= 19)
{
    // events 字段在 window 之后
    var eventsOffset = Marshal.OffsetOf<NNNativeEngineApi>("Events");
    var eventsPtr = (NNEventApi*)(nativeApiTable + eventsOffset);
    EventApi.Install(eventsPtr);
}
```

修改 `NNNativeEngineApi.cs`：

```csharp
[StructLayout(LayoutKind.Sequential)]
public struct NNNativeEngineApi
{
    public uint LayoutVersion;
    // ... existing fields ...
    public NNWindowApi Window;
    public NNEventApi Events;   // ← NEW (layout version 19+)
    // ... remaining fields ...
}
```

---

## 10. Editor Integration

### 10.1 Editor Event Flow

```
Native Event Queue
      │
      ▼
EditorMainLoop.PumpEvents()
      │
      ├── WindowClose → EditorWindowManager.CloseWindow()
      ├── WindowResized → EditorViewport.Resize()
      ├── DropFile → AssetImportPipeline.HandleDrop(path)
      ├── WindowFocusGained/Lost → EditorUI.FocusUpdate()
      ├── WindowDpiChanged → ImGui.SetDpiScale()
      └── Quit → EditorApplication.RequestQuit()
```

### 10.2 Editor Event Router

```csharp
/// <summary>
/// Editor 事件路由器——从 Native Event Queue 拉取事件，
/// 按类型分发到 Editor 子系统。
/// </summary>
public sealed class EditorEventRouter
{
    private readonly NNEvent[] _eventBuffer = new NNEvent[256];
    private readonly EditorWindowManager _windowManager;
    private readonly AssetImportPipeline _importPipeline;

    /// <summary>
    /// 每帧调用：批量消费 Native 事件并路由。
    /// </summary>
    public void PumpEvents()
    {
        int count = EventApi.PollEvents(_eventBuffer);

        for (int i = 0; i < count; i++)
        {
            ref readonly var evt = ref _eventBuffer[i];
            RouteEvent(in evt);
        }
    }

    private void RouteEvent(in NNEvent evt)
    {
        switch ((NNEventType)evt.Type)
        {
            // ─── Window ─────────────────────────
            case NNEventType.WindowClose:
                _windowManager.HandleClose(
                    new NNWindowHandle(evt.WindowHandle));
                break;

            case NNEventType.WindowResized:
            case NNEventType.WindowSizeChanged:
                _windowManager.HandleResize(
                    new NNWindowHandle(evt.WindowHandle),
                    evt.Data1, evt.Data2);
                break;

            case NNEventType.WindowMoved:
                _windowManager.HandleMove(
                    new NNWindowHandle(evt.WindowHandle),
                    evt.Data1, evt.Data2);
                break;

            case NNEventType.WindowFocusGained:
                _windowManager.HandleFocus(
                    new NNWindowHandle(evt.WindowHandle), true);
                break;

            case NNEventType.WindowFocusLost:
                _windowManager.HandleFocus(
                    new NNWindowHandle(evt.WindowHandle), false);
                break;

            case NNEventType.WindowDpiChanged:
                _windowManager.HandleDpiChange(
                    new NNWindowHandle(evt.WindowHandle),
                    evt.Data1 / 1000.0f);
                break;

            // ─── Drop ──────────────────────────
            case NNEventType.DropFile:
                if (EventApi.GetDropFilePath(in evt, out var filePath))
                {
                    _importPipeline.HandleFileDrop(
                        filePath,
                        new NNWindowHandle(evt.WindowHandle),
                        evt.Data1, evt.Data2);  // drop x, y
                }
                break;

            // ─── System ────────────────────────
            case NNEventType.Quit:
                EditorApplication.RequestQuit();
                break;

            default:
                // 未知事件 → 可选 log
                break;
        }
    }
}
```

### 10.3 Drop Asset 集成

```
用户拖拽文件到 Editor 窗口
      │
      ▼
SDL_EVENT_DROP_FILE
      │
      ▼
SDL3EventTranslator → NNEvent(DropFile, stringPoolIdx)
      │
      ▼
C# EventApi.GetDropFilePath()
      │
      ├── 已有 .meta → 打开资产
      ├── 在 Content 目录内 → 选中并高亮
      ├── 在 Content 外 → 复制到 Content/Imported/ 并 Import
      └── 多文件 Drop → 批量处理
```

### 10.4 ImGui Integration

ImGui 需要的事件通过 Layer 系统注入：

```
EditorMainLoop:
  1. PumpEvents()  → 消费 Native 事件
  2. RouteEvents() → 分发到子系统
  3. ImGui.NewFrame()
     ├── 注入 Resize → ImGui Display Size
     ├── 注入 Mouse Events → ImGui Input
     ├── 注入 Key Events → ImGui Input
     └── 注入 DPI Change → ImGui Font Scale
  4. EditorUI.Render()
  5. ImGui.Render()
```

**不修改现有 Layer 系统**。Editor 事件路由器在 `ImGui.NewFrame()` 前注入需要的数据。

---

## 11. Threading Model

### 11.1 线程约束

| 线程 | 角色 | 约束 |
|------|------|------|
| **主线程** | SDL_PollEvent + C# PollEvent | 唯一写入 Event Queue 的线程 |
| **主线程** | C# Event Pump | 唯一读取 Event Queue 的线程 |
| **其他线程** | 不接触 Event Queue | — |

### 11.2 为什么单线程就足够

- SDL 要求 `SDL_PollEvent` 只在创建窗口的线程调用（主线程）
- C# Editor 运行在主线程
- 同一线程既生产又消费 → **SPSC 天然满足**
- 无需 mutex、无需 condition variable（除 WaitEvent 的 spin）

### 11.3 多线程扩展（未来）

如果未来需要从其他线程推入事件（如 Network、Script）：

```cpp
// 方案：添加 mpscMultiProducerQueue → 主队列
class NNThreadSafeEventBridge
{
    // 其他线程写入此队列
    LockFreeMPSCQueue<NNEvent> m_pending;

    // 每帧开始时，主线程调用 DrainPending()
    void DrainPending(NNEventQueue& mainQueue)
    {
        NNEvent evt;
        while (m_pending.try_pop(evt))
        {
            mainQueue.Push(evt);
        }
    }
};
```

**当前阶段不需要**。先实现 SPSC 主线程方案。

---

## 12. Multi-Window Strategy

### 12.1 事件路由

每个 `NNEvent` 包含 `windowHandle` 字段，标识事件来源窗口。

```
C# Event Pump:
  while (PollEvent(out evt))
  {
      var handle = new NNWindowHandle(evt.WindowHandle);
      if (handle == editorMainWindow)
          editorMainWindow.HandleEvent(evt);
      else if (handle == viewportWindow)
          viewportWindow.HandleEvent(evt);
      // ...
  }
```

### 12.2 WindowRegistry 映射

Native 端维护 `SDL_WindowID → NNWindowHandle` 映射：

```cpp
// SDL3EventTranslator 内部
std::uint64_t LookupWindowHandle(std::uint32_t sdlWindowId) const noexcept
{
    // WindowRegistry 已有此映射
    return WindowRegistry::HandleFromSDLId(sdlWindowId);
}
```

### 12.3 全局事件（无窗口）

`SDL_EVENT_QUIT` 等无窗口事件的 `windowHandle = 0`。

---

## 13. ImGui Integration

### 13.1 ImGui 需要的事件

| ImGui 需求 | NNEvent 类型 | 数据 |
|-----------|-------------|------|
| Display Size | WindowResized | data1=width, data2=height |
| Mouse Position | MouseMotion (Future) | data1=x, data2=y |
| Mouse Buttons | MouseButton (Future) | data1=button |
| Keyboard | KeyDown/KeyUp (Future) | data1=scancode, data2=keycode |
| DPI Scale | WindowDpiChanged | data1=dpi×1000 |
| Focus | WindowFocusGained/Lost | — |
| Drop File | DropFile | stringPoolIdx → 路径 |

### 13.2 ImGui 注入方式

```csharp
// Editor 渲染循环
void OnFrame()
{
    // 1. 消费 Native 事件
    _router.PumpEvents();

    // 2. 注入到 ImGui（在 NewFrame 之前）
    var io = ImGui.GetIO();

    // Resize
    if (_latestResize.HasValue)
    {
        var (w, h) = _latestResize.Value;
        io.DisplaySize = new Vector2(w, h);
        io.DisplayFramebufferScale = Vector2.One;
    }

    // DPI
    if (_latestDpi.HasValue)
    {
        io.FontGlobalScale = _latestDpi.Value;
    }

    // Mouse/Keyboard — 通过现有 ImGui Layer 系统
    // 或通过 ImGuiBackend（SDL3 backend 直接处理）

    // 3. ImGui 渲染
    ImGui.NewFrame();
    // ... Editor UI ...
    ImGui.Render();
}
```

### 13.3 与现有 Layer 系统的关系

**不替换** 现有的 `SDL3Window::Layer` 系统。ImGui Layer 继续直接处理 SDL 输入事件（通过 SDL3 ImGui Backend）。

Event Pump 是**额外**的通道，供 Editor 业务逻辑使用（Resize 处理、Drop 导入、Focus 管理等）。ImGui 输入仍走现有 Layer 流。

---

## 14. Future-Proof Design

### 14.1 预留位分析

| 预留 | 用途 |
|------|------|
| `NNEvent.reservedPad[12]` (48B) | 未来 Input 事件字段（scancode, keycode, mods, gamepad 等） |
| `NNEventType` 范围块 | 每类事件 256 个值，远超当前需求 |
| `NNEventCategory` 32-bit | 仅用 6 bit，26 bit 预留 |
| `NNEvent.flags` | 事件特定标志（如 KeyDown 的 repeat flag） |
| `NNEventAPI` 的 `pushUserEvent` | C# → Native 用户事件 |
| `NNEventType::UserEvent` (0x8000) | 引擎内部扩展事件 |

### 14.2 未来扩展路径

| 功能 | 如何扩展 |
|------|---------|
| **Input System** | 新增 `NNInputEvent` 或复用 `NNEvent`（使用 reservedPad） |
| **Game Runtime** | 同一 Event Queue，Game 层消费 |
| **Remote Editor** | Event Queue 序列化到 TCP/WebSocket → Remote 端 PollEvent |
| **Network Events** | `UserEvent` (0x8000) + 自定义 subtype |
| **Script Events** | Script Runtime 通过 `pushUserEvent` 推送 |
| **Web Platform** | Emscripten 事件 → 同一 NNEvent 翻译 |
| **Mobile** | 触摸事件 → Mouse 事件 + Touch 扩展 |

### 14.3 ABI 版本策略

```
Layout Version:
  18 → 现有（无 Event API）
  19 → 添加 NNEventAPI
  20 → 添加 Input 字段到 NNEvent.reservedPad
  21 → 添加 Touch 事件类别
  ...

规则:
  - 新 API 表追加到 NNNativeEngineAPI 末尾
  - 新事件类型追加到 NNEventType 末尾
  - NNEvent.reservedPad 逐步激活（从末尾往前用）
  - Layout Version 只增不减
  - C# 端 LayoutVersion 检测可用功能
```

### 14.4 事件大小不变保证

```
NNEvent = 128B，永不改变。

如果未来需要更多数据（如 Gamepad 完整状态）：
  方案 A: 新增 NNGamepadStateEvent (256B) 作为独立结构体
  方案 B: 使用 UserEvent + 外部共享内存
  方案 C: 新增 NNExtendedEvent (256B) + 事件类型区分

核心原则：
  NNEvent 本身 = 128B = 永久 ABI 稳定
  扩展 = 新结构体 + 新 API，而非修改旧结构体
```

---

## 15. File Change Checklist

### 15.1 C++ Native Side

| 文件 | 操作 | 说明 |
|------|------|------|
| `NNNativeEngineAPI/Include/EventTypes.h` | **新建** | `NNEventType`, `NNEventCategory`, `NNEvent` |
| `NNNativeEngineAPI/Include/EventAPI.h` | **新建** | `NNEventAPI` 函数表 |
| `NNNativeEngineAPI/Include/EngineAPIRegistry.h` | 修改 | 添加 `const NNEventAPI* events` 字段 |
| `NNRuntimeApplication/Include/Core/EventQueue.h` | **新建** | `NNEventQueue` (SPSC ring buffer + string pool) |
| `NNRuntimeApplication/Private/SDL3EventTranslator.h` | **新建** | `SDL3EventTranslator` 类 |
| `NNRuntimeApplication/Private/BuildEventApi.cpp` | **新建** | `NNBuildEventRuntimeApi()` |
| `NNRuntimeApplication/Include/Core/RuntimeApplication.h` | 修改 | 添加 `NNEventQueue* m_eventQueue` |
| `NNRuntimeApplication/Private/RuntimeApplication.cpp` | 修改 | PumpEvents 改用 EventQueue |
| `NNRuntimeNativeEngineAPIStub/.../EventApiStubs.cpp` | **新建** | 测试用 stub 实现 |

### 15.2 C# Managed Side

| 文件 | 操作 | 说明 |
|------|------|------|
| `Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | 修改 | 添加 `NNEvent`, `NNEventType`, `NNEventApi` |
| `Neverness.Runtime.Engine/NNNativeEngineApiConstants.cs` | 修改 | `LayoutVersion = 19` |
| `Neverness.Runtime.Engine/NNNativeEngineApi.cs` | 修改 | 添加 `Events` 字段 |
| `Neverness.Runtime.Application/EventApi.cs` | **新建** | `EventApi` 高层封装 |
| `Neverness.Editor/EditorEventRouter.cs` | **新建** | Editor 事件路由 |
| `Tests/NativeEngineApiEventTests.cs` | **新建** | ABI 验证测试 |

### 15.3 CMake

| 文件 | 操作 |
|------|------|
| `NNRuntimeApplication/CMakeLists.txt` | 添加新源文件 |
| `NNRuntimeNativeEngineAPIStub/CMakeLists.txt` | 添加 stub 源文件 |

---

## 16. Implementation Phases

### Phase 1: ABI 定义（本次）

- 新建 `EventTypes.h` (`NNEventType`, `NNEventCategory`, `NNEvent`)
- 新建 `EventAPI.h` (`NNEventAPI`)
- 修改 `EngineAPIRegistry.h` 添加 `events` 字段
- C# 新建对应结构体
- C# `LayoutVersion` 升级到 19
- ABI 验证测试：`Marshal.SizeOf<NNEvent>() == 128`

### Phase 2: Event Queue 实现

- 新建 `EventQueue.h` (SPSC ring buffer + string pool)
- 新建 `SDL3EventTranslator.h`
- 新建 `BuildEventApi.cpp`
- 修改 `RuntimeApplication::PumpEvents()` 推入队列
- C# `EventApi.cs` 封装

### Phase 3: Editor 集成

- `EditorEventRouter` 消费事件
- Drop File → Asset Import
- Window Resize → Viewport 更新
- Window Close → 安全关闭
- DPI Change → ImGui font scale
- Focus → Editor 状态管理

### Phase 4: Input 扩展（未来）

- 鼠标事件翻译
- 键盘事件翻译
- `NNEvent.reservedPad` 激活 Input 字段
- ImGui Input 后端切换到 Event Pump

---

## Appendix A: Memory Layout Verification

```
NNEvent (128 bytes):

  0x00: type          [4B]  uint32
  0x04: category      [4B]  uint32
  0x08: timestamp     [8B]  uint64
  0x10: windowHandle  [8B]  uint64
  0x18: data1         [4B]  int32
  0x1C: data2         [4B]  int32
  0x20: data3         [4B]  int32
  0x24: data4         [4B]  int32
  0x28: flags         [4B]  uint32
  0x2C: reserved0     [4B]  uint32
  0x30: stringPoolIdx [4B]  uint32
  0x34: reservedPad   [48B] uint32[12]
  ---
  Total: 4+4+8+8+4+4+4+4+4+4+4+48 = 100 + 28 = 128B ✓
```

```
NNEventAPI (x64):

  offset 0:   size              [4B + 4B pad]
  offset 8:   pollEvent         [8B]  fn ptr
  offset 16:  peekEvent         [8B]  fn ptr
  offset 24:  waitEvent         [8B]  fn ptr
  offset 32:  getEventString    [8B]  fn ptr
  offset 40:  getQueueCount     [8B]  fn ptr
  offset 48:  flushEvents       [8B]  fn ptr
  offset 56:  pushUserEvent     [8B]  fn ptr
  ---
  Total: 64B on x64
```

## Appendix B: String Pool 安全性分析

### 场景：极端 Drop 洪泛

假设用户快速拖拽 1000 个文件到窗口：
- 每个 DropFile 事件：~300 字节路径
- 1000 × 300 = 300KB >> 64KB String Pool

**缓解措施**：
1. 每帧消费上限 256 事件 → 最多消费 ~256 × 300 = 76.8KB
2. C# 端每帧先消费再处理（不是边消费边处理）
3. String Pool 64KB 可存 ~213 条 300 字节路径
4. 如果 Pool 满 → 丢弃事件（不丢数据完整性，只是少处理几个 drop）

### 场景：Resize spam

用户拖拽窗口边框调整大小：
- SDL 每帧产生 1-5 个 RESIZE 事件
- Ring Buffer 4096 → 可缓冲 ~800 帧（13 秒 @ 60fps）
- `TryCoalesceResize()` 合并同一帧内的连续 resize

**结论**：安全性充分。

---

## Appendix C: 性能估算

| 操作 | 耗时估算 | 说明 |
|------|----------|------|
| `Push(NNEvent)` | ~5ns | 原子 store + memcpy 128B |
| `Pop(NNEvent)` | ~5ns | 原子 load + memcpy 128B |
| `WriteString()` (300B) | ~15ns | memcpy 300B + 2B length |
| `SDL3EventTranslator` | ~20ns/事件 | switch + field copy |
| `PollEvents(256)` | ~5μs | 256 × (5 + 20) ns |
| C# `PollEvent()` | ~30ns | P/Invoke overhead (indirect call) |
| C# `PollEvents(256)` | ~8μs | 256 × 30ns |

**总开销 < 20μs/frame** — 可忽略。
