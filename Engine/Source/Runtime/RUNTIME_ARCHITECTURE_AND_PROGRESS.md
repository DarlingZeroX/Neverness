# Neverness Runtime 架构文档

> 最后更新：2026-06-22

---

## 目录

- [1. 概述](#1-概述)
- [2. 模块总览](#2-模块总览)
- [3. 活跃模块详解](#3-活跃模块详解)
  - [3.1 NNAPI — ABI 契约层](#31-nnapi--abi-契约层)
  - [3.2 NNRuntimeCore — 核心运行时框架](#32-nnruntimecore--核心运行时框架)
  - [3.3 NNRuntimeEngine — 引擎运行时门面](#33-nnruntimeengine--引擎运行时门面)
  - [3.4 NNRuntimeApplication — 应用宿主层](#34-nnruntimeapplication--应用宿主层)
  - [3.5 NNRuntimeVFS — 虚拟文件系统](#35-nnruntimevfs--虚拟文件系统)
  - [3.6 NNRuntimeLua — Lua 脚本运行时](#36-nnruntimelua--lua-脚本运行时)
  - [3.7 NNRuntimeImGui — ImGui 扩展模块](#37-nnruntimeimgui--imgui-扩展模块)
  - [3.8 NNRuntimeRenderer2D — 2D 精灵渲染器](#38-nnruntimerenderer2d--2d-精灵渲染器)
  - [3.9 NNRuntimeRmlui — RmlUI 集成](#39-nnruntimermlui--rmlui-集成)
  - [3.10 NNRuntimePak — Pak 打包系统](#310-nnruntimepak--pak-打包系统)
  - [3.11 NNRuntimeMedia — 音视频播放](#311-nnruntimemedia--音视频播放)
  - [3.12 NNRuntimeEngineServices — 引擎服务桥接](#312-nnruntimeengineservices--引擎服务桥接)
- [4. Legacy 模块（已废弃）](#4-legacy-模块已废弃)
- [5. 依赖关系图](#5-依赖关系图)
- [6. ABI 层架构](#6-abi-层架构)
- [7. 构建系统](#7-构建系统)
- [8. 当前状态与进度](#8-当前状态与进度)

---

## 1. 概述

Neverness Runtime 是 Neverness 引擎（内部代号 VisionGal — 视觉小说引擎）的运行时层，由 C++17 编写，通过 C ABI 函数指针表与 C# 托管代码互操作。

**设计原则：**

- **ABI 稳定**：所有跨 DLL/跨语言边界通过 `extern "C"` 函数指针表传递，避免 C++ name mangling 和 vtable ABI 问题
- **依赖倒置**：Core 模块定义纯虚接口（`I*` 前缀），具体实现在下游模块
- **Handle 化**：所有跨边界的对象引用使用 `uint64_t` opaque handle，不暴露 C++ 指针
- **模块独立**：每个模块有独立的 CMakeLists.txt、导出宏、Public/Private 头文件约定

**技术栈：**

| 技术 | 用途 |
|------|------|
| SDL3 | 窗口管理、输入事件、音频输出 |
| Diligent Engine | GPU 渲染后端（替代 OpenGL） |
| RmlUI | HTML/CSS UI 中间件 |
| ImGui 1.92.3 | 编辑器 UI |
| Lua 5.4.8 + sol2 | 脚本系统 |
| entt | ECS 实体组件系统 |
| FFmpeg | 视频解码 |
| miniz | ZIP 归档读取 |
| Zstd/LZ4 | Pak 压缩 |
| Freetype | 字体渲染 |

---

## 2. 模块总览

### 活跃模块（当前构建参与）

| 模块 | CMake 目标 | 类型 | 职责 |
|------|-----------|------|------|
| **NNAPI** | `NevernessRuntime-API` | STATIC | ABI 契约 + Stub 实现 |
| **NNRuntimeCore** | `NevernessRuntime-Core` | SHARED | 核心接口、事件、输入、视口 |
| **NNRuntimeEngine** | `NevernessRuntime-Engine` | STATIC | 引擎生命周期、调度器、计时、异步、对象表 |
| **NNRuntimeApplication** | `NevernessRuntime-Application` | STATIC | SDL3 宿主、窗口、事件队列、ImGui 层 |
| **NNRuntimeVFS** | `NevernessRuntime-VFS` | SHARED | 虚拟文件系统（Native/Memory/Zip） |
| **NNRuntimeLua** | `NevernessRuntime-Lua` | SHARED | Lua 5.4.8 VM |
| **NNRuntimeImGui** | `NevernessRuntime-ImGui` | SHARED | ImGui 核心 + 扩展 |
| **NNRuntimeRenderer2D** | `NevernessRuntime-Renderer2D` | SHARED | 2D 精灵批量渲染 |
| **NNRuntimeRmlui** | `NevernessRuntime-RmlUI` | SHARED | RmlUI 文档管理 + 渲染 |
| **NNRuntimePak** | `NevernessRuntime-Pak` | SHARED | .pak 打包/解包 + VFS 挂载 |
| **NNRuntimeMedia** | `NevernessRuntime-Media` | SHARED | 音频/视频播放 |
| **NNRuntimeEngineServices** | `NevernessRuntime-EngineServices` | SHARED | ABI 转发层，桥接 Stub → 真实实现 |

### Legacy 模块（已废弃/冻结）

| 模块 | 状态 | 替代方案 |
|------|------|---------|
| NNEngineLegacy | 冻结 | 拆分为独立模块 |
| NNRuntimeScene | Legacy | C# Friflo ECS |
| NNRuntimeAsset | Legacy | C# AssetManager |
| NNRuntimeAssetLegacy | Legacy | C# 资产系统 |
| NNRuntimeRHI | 已禁用 | NNRuntimeDiligent |
| NNRuntimeRenderAssets | Legacy | C# 资产管线 |
| NNRuntimeImageCodec | Legacy | stb_image 直接使用 |
| NNRuntimeManagedBridge | Legacy | Entry.Bootstrap |
| NNRuntimeManagedHostLegacy | Legacy | 产品 hostfxr |
| NNRuntimeMediaAssets | 待修复 | — |

---

## 3. 活跃模块详解

### 3.1 NNAPI — ABI 契约层

**路径：** `Runtime/NNAPI`
**目标：** `NevernessRuntime-API` (STATIC)
**依赖：** 无（纯头文件 + Stub）

#### 职责

定义引擎暴露给 C# 托管代码的完整 C ABI 函数指针表。Stub 实现只做日志记录不做任何事，真实实现在 NNRuntimeEngineServices。

#### 核心类型

```
NNNativeAPI                          // 顶层导出表，C# 拿到的就是这个
├── apiVersion (uint32_t, 当前 3)
├── reserved0
└── const NNNativeEngineAPI* engineServices
    ├── NNRenderAPI                  // createTexture, uploadTexture, createRenderTarget
    ├── NNAudioAPI                   // 音频控制
    ├── NNInputAPI                   // 输入查询
    ├── NNAsyncWaitAPI               // 异步等待
    ├── NNApplicationAPI             // 应用生命周期
    ├── NNWindowAPI                  // 窗口操作
    ├── NNVfsAPI                     // 虚拟文件系统
    ├── NNEventAPI                   // 事件轮询
    ├── NNRenderAssetAPI             // 渲染资产
    ├── NNViewportRenderAPI          // 视口渲染
    ├── NNViewportSurfaceAPI         // 视口表面
    └── NNDiligentAPI                // Diligent 设备访问
```

#### Handle 类型（全部 `uint64_t`）

| Handle | 用途 |
|--------|------|
| `NNTextureHandle` | GPU 纹理 |
| `NNRenderTargetHandle` | 渲染目标 |
| `NNElementHandle` | UI 元素 |
| `NNAudioHandle` | 音频实例 |
| `NNAssetHandle` | 资产引用 |
| `NNAsyncWaitHandle` | 异步等待 |
| `NNEntityHandle` | ECS 实体 |

#### RenderCommands — 二进制命令缓冲

C# → C++ 渲染数据传递使用紧凑的二进制命令缓冲格式：

| 命令码 | 类型 | 大小 |
|--------|------|------|
| `0x01` | `SetCamera` | `NNSetCameraData` |
| `0x02` | `SetRenderPassState` | `NNRenderPassStateData` |
| `0x10` | `DrawSpriteBatch` | `NNSpriteInstance[]` (120 bytes/个) |
| `0x20` | `SetRmlDocuments` | `NNRmlDocumentEntry[]` (272 bytes/个) |

---

### 3.2 NNRuntimeCore — 核心运行时框架

**路径：** `Runtime/NNRuntimeCore`
**目标：** `NevernessRuntime-Core` (SHARED)
**导出宏：** `NN_RUNTIME_CORE_API`

#### 职责

所有上层运行时模块的基础抽象层。定义接口契约，具体实现交给下游模块。

#### 三层架构

```
Include/Core/       ← 具体实现：RuntimeCore、Input、Viewport、EventBus、Events
Interface/          ← 纯虚接口：I* 前缀，依赖倒置
Include/Data/       ← 数据持久化：VGDataVariant（Lua 绑定 + JSON 序列化）
Include/Utils/      ← 工具函数：LuaHelper、TimeHelper、TransitionHelper
```

#### 核心类型

**基础设施：**

| 类型 | 说明 |
|------|------|
| `RuntimeCore` | 静态单例：初始化、时间、文件接口、VFS 路径解析 |
| `VGObject` | 所有引擎对象基类（虚析构 + `ToString()`） |
| `VGEngineResource` | 资源基类，携带资源路径 |

**事件系统 (`EngineEventBus` 单例)：**

| 事件通道 | 事件类型 |
|----------|---------|
| SceneEvent | Actor 创建/移除/选中 |
| EngineEvent | 主场景变更、进入/退出 Play Mode |
| LuaScriptEvent | 脚本错误（路径、消息、行号） |
| UISystemEvent | UI 文件打开/关闭 |
| ViewportEvent | 尺寸变更、悬停、焦点、鼠标、拖放 |

**Scene/ECS 接口（基于 entt）：**

| 接口 | 说明 |
|------|------|
| `IEntity` | 实体基类，包装 `entt::entity` |
| `IGameActor` | 游戏实体：标签、可见性、组件增删查 |
| `IComponent` | 组件基类：类型字符串 + cereal 序列化 |
| `IScene` | 场景接口：Actor CRUD、实体-组件管理、更新 |

**脚本接口：**

| 接口 | 说明 |
|------|------|
| `IScript` | 生命周期：`Awake` → `Start` → `Update` → `FixedUpdate` |
| `IScriptVariable` | 可序列化脚本变量（EntityID、Path、String、Bool、Double） |
| `IAnimationScript` | 动画脚本：`Start`、`IsFinished`、`OnUpdate` |

**数据系统：**

| 类型 | 说明 |
|------|------|
| `VGDataVariant` | 变体类型（Nil/Bool/Int/Num/String/Table），sol2 Lua 互操作 + JSON |
| `VGDataNamespace` | 命名键值存储 |
| `VGDataContainer` | 命名空间容器 |

#### 依赖

| 依赖 | 链接方式 | 用途 |
|------|---------|------|
| NNRuntimeLua | PRIVATE | sol2/Lua 绑定 |
| NNCore | PRIVATE | 核心类型、事件委托、UUID、JSON |
| NNPlatformCore | PRIVATE | SDL3 窗口/输入抽象 |
| NNRuntimeVFS | PRIVATE | 虚拟文件系统 |
| SDL3 | PUBLIC | 窗口管理、输入 |

---

### 3.3 NNRuntimeEngine — 引擎运行时门面

**路径：** `Runtime/NNRuntimeEngine`
**目标：** `NevernessRuntime-Engine` (STATIC)

#### 职责

进程级引擎运行时门面，提供核心游戏循环骨架：**Initialize → Tick → Shutdown**。

#### 四个子系统

```
NNEngineRuntime (单例门面)
├── TimingSystem        ← 帧计时：deltaTime、totalTime、frameIndex
├── AsyncSystem         ← 可轮询异步等待（std::thread 后台睡眠）
├── ObjectSubsystem     ← 原生侧引用计数对象表（NNObjectHandle）
└── RuntimeScheduler    ← Unity PlayerLoop 风格的 Tick 调度器
```

#### Tick 调度顺序

```
每帧：
  EarlyUpdate
  → FixedUpdate × (0..N)     // 默认 50Hz，每帧最多 5 次，防螺旋死亡
  → Update
  → LateUpdate
  → FlushMainThreadDelegates  // 占位符
  → Render
```

#### 核心类型

| 类型 | 说明 |
|------|------|
| `NNEngineRuntime` | 单例门面：`Instance()`、`Initialize()`、`Tick(dt)`、`Shutdown()` |
| `TimingSystem` | `GetDeltaTime()`、`GetTotalTime()`、`GetFrameIndex()` |
| `AsyncSystem` | `CreateWait()` → handle，`TryComplete(handle)`，`ReleaseWait(handle)` |
| `ObjectSubsystem` | `CreateObject(typeName)` → handle，引用计数管理，线程安全 |
| `IRuntimeSubsystem` | 实现此接口注册到调度器：`Tick(context)` + `TickGroup()` |
| `RuntimeTickGroup` | 枚举：EarlyUpdate / FixedUpdate / Update / LateUpdate / Render |
| `RuntimeFrameContext` | POD：deltaTimeSeconds、totalTimeSeconds、frameIndex、fixedDeltaTimeSeconds |

#### 依赖

仅依赖 `NevernessRuntime-NativeEngineAPI`（Handle 定义），无外部第三方依赖。

---

### 3.4 NNRuntimeApplication — 应用宿主层

**路径：** `Runtime/NNRuntimeApplication`
**目标：** `NevernessRuntime-Application` (STATIC)

#### 职责

SDL3 子系统生命周期管理、原生 OS 窗口创建（SDL3 + Diligent GPU 设备/交换链）、事件翻译与传递管线、ImGui 集成。

#### 架构

```
RuntimeApplication (中心宿主)
├── SDL3 子系统初始化/关闭
├── EventQueue (SPSC 无锁环形缓冲, 4096 槽 × 128 字节 + 64KB 字符串池)
├── SDL3EventTranslator (SDL_Event → NNEvent ABI 稳定结构体)
├── WindowRegistry (NNWindowHandle → VGWindow 映射)
├── VGWindow (SDL3 窗口 + Diligent 交换链)
└── ImguiDiligentLayer (ImGui Diligent 后端)
```

#### 暴露的 C ABI 函数表

| API 表 | 功能 |
|--------|------|
| `NNApplicationAPI` | initialize / pumpEvents / beginFrame / endFrame / shutdown |
| `NNWindowAPI` | create / destroy / setTitle / setSize / maximize / minimize / ... |
| `NNEventAPI` | pollEvent / peekEvent / waitEvent / getEventString / pushUserEvent |

#### 事件系统

SDL3 事件通过 `SDL3EventTranslator` 翻译为 ABI 稳定的 `NNEvent` POD 结构体（128 字节），使用两级 type+subtype 方案：

| 事件类别 | 子类型数量 |
|----------|-----------|
| 窗口事件 | 17 种（尺寸、位置、焦点、最小化...） |
| 输入事件 | 鼠标移动/按钮/滚轮、键盘按下/释放、文本输入/编辑、拖放 |
| 系统事件 | 退出、终止、低内存 |

#### 依赖

| 依赖 | 用途 |
|------|------|
| NNAPI (PUBLIC) | ABI 类型定义 |
| SDL3 (PUBLIC) | 窗口/输入/事件 |
| NNRuntimeCore | 核心服务 |
| NNRuntimeVFS | 虚拟文件系统 |
| NNRuntimeImGui | ImGui 核心 |
| NNRuntimeDiligent | Diligent 渲染后端 |

---

### 3.5 NNRuntimeVFS — 虚拟文件系统

**路径：** `Runtime/NNRuntimeVFS`
**目标：** `NevernessRuntime-VFS` (SHARED)
**导出宏：** `NN_RUNTIME_VFS_API`

#### 职责

统一的文件访问抽象层，支持多种存储后端（磁盘、内存、ZIP 归档），通过路径别名路由文件请求。

#### 后端实现

| 后端 | 类 | 读写 | 说明 |
|------|-----|------|------|
| Native | `NativeFileSystem` / `NativeFile` | 读写 | `std::fstream`，磁盘目录 |
| Memory | `MemoryFileSystem` / `MemoryFile` | 读写 | `std::vector<uint8_t>`，纯内存 |
| Zip | `ZipFileSystem` / `ZipFile` | 只读 | miniz，ZIP 归档 |

#### 核心架构

```
VirtualFileSystem (中央多路复用器)
├── alias → [IFileSystemPtr, ...] 映射
├── 别名按最长前缀匹配排序
└── OpenFile() 遍历别名 → 剥离前缀 → 尝试每个后端

VFSService (静态服务层)
├── GetInstance() → 全局 VirtualFileSystem
├── MountFileSystem(alias, fs)
├── ReadTextFromFile / WriteTextToFile / WriteBufferToFile
├── AddFileSystem(alias, type, path) → handle (C# 互操作)
└── NNBuildVfsRuntimeApi() → NNVfsAPI 函数表
```

#### 线程安全

编译时开关 `VFS_ENABLE_MULTITHREADING`，启用后所有 VirtualFileSystem 方法加 mutex 保护。

#### 依赖

| 依赖 | 用途 |
|------|------|
| NNAPI (PUBLIC) | `NNVfsAPI` 结构体定义 |
| NNCore (PRIVATE) | `NN::Ref`、日志宏 |
| NNPlatformCore (PRIVATE) | 路径工具 |
| miniz (内嵌) | ZIP 读取 |

---

### 3.6 NNRuntimeLua — Lua 脚本运行时

**路径：** `Runtime/NNRuntimeLua`
**目标：** `NevernessRuntime-Lua` (SHARED)

#### 职责

打包 Lua 5.4.8（ANSI C）为共享 DLL，附带错误行号跟踪和本地化工具函数。包含 sol2 兼容性头文件。

#### 内容

- Lua 5.4.8 完整 VM（C90 编译）
- `VGLuaCoreSetErrorLineNumber()` / `VGLuaCoreGetErrorLineNumber()` — Lua 错误行号跟踪
- `VGLuaCoreLocalize()` — Lua 消息本地化
- sol2 兼容性头文件（compat-5.3/5.4 shims）

#### 依赖

仅依赖 `NevernessCore-Core`。

---

### 3.7 NNRuntimeImGui — ImGui 扩展模块

**路径：** `Runtime/NNRuntimeImGui`
**目标：** `NevernessRuntime-ImGui` (SHARED)

#### 职责

ImGui 1.92.3 核心编译 + 丰富扩展集。正在从 OpenGL3 后端迁移到 Diligent 后端。

#### 扩展列表

| 扩展 | 说明 |
|------|------|
| ImGuiColorTextEdit | 代码文本编辑器 |
| ImNodeEditor | 节点编辑器 |
| Imnodes | 轻量节点编辑器 |
| ImGuizmo | 3D Gizmo 操作 |
| FontAwesome / MaterialDesign / Kenney 图标字体 | 图标集 |
| ImWindow / IDockSpaceWindow | 窗口/Dock 框架 |
| ImTaskManager | 后台任务管理器 |
| ImFontManager | 字体图集管理器 |
| ImToast | Toast 通知系统 |

#### 核心类型

| 类型 | 说明 |
|------|------|
| `ImWindow` | 窗口基类：`OnGUI()`、`OnWindowGUI()` |
| `IDockSpaceWindow` | Dock 窗口，通过 `InsertWindow()` 托管子窗口 |
| `ImPanelInterface` | 面板接口：`FrameUpdate()`、`OnGUI()`、`IsAsync()` |
| `ImTaskInterface` | 后台任务：进度、强制停止、完成状态 |
| `ImTaskManager` | 单例任务管理器 |
| `ImFontManager` | 单例字体管理器 |

#### 依赖

| 依赖 | 用途 |
|------|------|
| Freetype | 字体渲染 |
| SDL3 | 平台后端 |
| NNRuntimeDiligent | Diligent 渲染后端（迁移中） |
| NNRuntimeRender | 渲染设备接口 |

---

### 3.8 NNRuntimeRenderer2D — 2D 精灵渲染器

**路径：** `Runtime/NNRuntimeRenderer2D`
**目标：** `NevernessRuntime-Renderer2D` (SHARED)

#### 职责

极简 2D 精灵批量渲染管线。接受 `SpriteDrawCommand` 数组，通过 Diligent 后端批量绘制四边形。

#### 核心类型

| 类型 | 说明 |
|------|------|
| `Renderer2D` | 主渲染器：`Initialize` → `BeginScene` → `Submit` → `EndScene` |
| `CameraData` | 相机 MVP 矩阵（4×4 列主序）、正交宽高、近远平面 |
| `FramebufferObject` | 离屏渲染目标，`GetColorTextureHandle()` 返回 ImGui 可用的 handle |
| `BuiltinShaders` | 内联 HLSL 着色器：MVP 变换、UV 矩形映射、翻转、颜色着色、linear→sRGB |

#### 渲染流程

```
BeginScene(CameraData, width, height)  ← 设置 VP 矩阵、绑定 RTV/DSV
  Submit(vector<SpriteDrawCommand>)    ← 收集精灵实例
EndScene()                             ← 批量绘制、sRGB 转换
```

#### 依赖

`NNRuntimeRender`、`NNRuntimeDiligent`、`NevernessCore-Core`。

---

### 3.9 NNRuntimeRmlui — RmlUI 集成

**路径：** `Runtime/NNRuntimeRmlui`
**目标：** `NevernessRuntime-RmlUI` (SHARED)

#### 职责

RmlUI（HTML/CSS UI 中间件）集成模块。管理文档生命周期，通过 Diligent 后端渲染。

#### 双层架构

```
系统层 (NNRmlUISystem)
├── Tick(deltaTime)
├── SetDrawItems(vector<RmlDrawItem>)  ← 纯数据，无 ECS 依赖
└── GetDrawList() → RmlDrawList

渲染层 (RmlUIRenderer)
├── Initialize(INNRenderDevice*, w, h)
├── Sync(DrawList)                     ← 同步文档状态
├── Update()                           ← RmlUI 逻辑更新
├── Render(DrawList, ViewTarget)       ← 渲染到屏幕
├── RenderToTexture(...)               ← 渲染到纹理
└── ProcessInput(...)                  ← 输入转发
```

#### 核心类型

| 类型 | 说明 |
|------|------|
| `RmlDrawItem` | 纯数据：entity、assetGuid、assetPath、sortOrder、viewTarget |
| `NNRmlUIViewTarget` | 枚举：Scene / Game / Both |
| `RmlDocumentRuntime` | 每文档状态：`Rml::ElementDocument*`、状态（Ready/Hidden/Failed） |
| `IRmlUIAssetResolver` | GUID→路径解析接口 |

#### 依赖

`RmlUi`、`SDL3`、`NNRuntimeRender`、`NNRuntimeDiligent`、`NNRuntimeLua`、`NNRuntimeCore`、`NNRuntimeVFS`。

---

### 3.10 NNRuntimePak — Pak 打包系统

**路径：** `Runtime/NNRuntimePak`
**目标：** `NevernessRuntime-Pak` (SHARED)

#### 职责

自定义 `.pak` 打包格式，用于捆绑游戏资产。支持 Zstd/LZ4 压缩和 AES 加密。

#### Pak 文件格式

```
PakHeader (Magic: "VGPC", major/minor version, index offset)
PakEntry[] (offset, encryption, compression, compressed/original size, CRC32, path)
FileData[] (实际文件数据)
```

#### 核心类型

| 类型 | 说明 |
|------|------|
| `PakFileReader` | 读取 pak 文件、解压条目、CRC32 校验 |
| `PakFileWriter` | 写入 pak 文件、压缩、加密 |
| `VGPackageFileSystem` | 实现 `IFileSystem`，可挂载到 VFS |
| `VGPackageFile` | 实现 `IFile`，内存流读取 |
| `MountPackageFileSystem()` | 便捷挂载函数 |

#### 依赖

`SDL3`、`NevernessCore-Core`、`NevernessRuntime-VFS`。

---

### 3.11 NNRuntimeMedia — 音视频播放

**路径：** `Runtime/NNRuntimeMedia`
**目标：`NevernessRuntime-Media` (SHARED)

#### 职责

音频播放（SDL3 音频输出）、视频播放（FFmpeg 解码 + SDL3 音频 + GPU 纹理更新）、视频纹理管理。

#### 核心类型

| 类型 | 说明 |
|------|------|
| `NNAudioPlayer` | `Play/Stop/Pause/Resume/Seek`，音高/音量/循环控制 |
| `NNVideoPlayer` | `Play/Stop/Update(deltaTime)`，每帧解码 + 纹理更新 |
| `NNVideoTexture` | RGBA 帧 → GPU 纹理桥接，`GetImGuiHandle()` |
| `NNAudioClipAsset` | 运行时音频资产：PCM 数据、采样率、声道、时长 |
| `NNVideoClipAsset` | 运行时视频资产：RGBA 帧、宽高、FPS、时长 |
| `NNMediaRuntimeAPI` | C ABI 函数指针表，供 C# P/Invoke |

#### 状态

部分源文件仍为 TODO（AudioClipAsset.cpp、VideoClipAsset.cpp、VideoPlayer.cpp）。

#### 依赖

`NevernessCore-Core`、`NevernessCore-MediaCore`、`NevernessRuntime-VFS`。

---

### 3.12 NNRuntimeEngineServices — 引擎服务桥接

**路径：** `Runtime/NNRuntimeEngineServices`
**目标：** `NevernessRuntime-EngineServices` (SHARED)

#### 职责

ABI 运行时转发层。将 NNAPI 的 Stub 定义桥接到真实的引擎运行时实现。

#### 导出函数

| 函数 | 说明 |
|------|------|
| `NNNativeApi_GetDefaultTable()` | 返回进程级单例 `NNNativeAPI*` |
| `NNNativeApiTable_BuildDefault()` | 填充函数指针表为真实实现 |
| `NNNativeEngineApiTable_BuildRuntime()` | 覆写 Timing/Async/Scene/Asset 字段 |
| `NNEngineRuntimeHost_Initialize()` | 引擎初始化 |
| `NNEngineRuntimeHost_Tick()` | 引擎 Tick |
| `NNEngineRuntimeHost_Shutdown()` | 引擎关闭 |

#### 内部源文件

- `NativeEngineRuntimeApiTable.cpp` — 运行时转发
- `AsyncWaitRuntimeApi.cpp` — 异步等待实现
- `ViewportRenderRuntimeApi.cpp` — 视口渲染实现
- `ViewportSurfaceRuntimeApi.cpp` — 视口表面实现
- `DiligentRuntimeApi.cpp` — Diligent 设备访问
- `ManagedExports.cpp` — 托管导出
- `ManagedRuntimeServices.cpp` — 托管运行时服务

#### 依赖

| 依赖 | 链接方式 |
|------|---------|
| NNAPI | PUBLIC |
| NNRuntimeEngine | PRIVATE |
| NNRuntimeApplication | PRIVATE |
| NNRuntimeVFS | PRIVATE |
| NNRuntimeRmlui | PRIVATE |
| NNRuntimeDiligent | PRIVATE |
| NNRuntimeRenderer2D | PRIVATE |

---

## 4. Legacy 模块（已废弃）

### 4.1 NNEngineLegacy

原单体引擎模块。包含动画系统、引擎管理器、游戏/渲染管线、Lua 接口、渲染原语、场景系统。**已冻结**，缺少 `NNRuntimeRmlui/Interface/UIDocumentLegacy.h`。

### 4.2 NNRuntimeScene

ECS 优先的运行时场景图。实体/世界管理、组件系统（Transform、Relationship、Tag、Camera、SpriteRenderer、AudioSource、Script、RmlUIDocument、VideoPlayer）、场景系统、JSON 序列化器、Prefab 支持。**已被 C# Friflo ECS 替代**。

### 4.3 NNRuntimeAsset

运行时资产管理系统。资产管理器、Handle 表、资产缓存、流式加载、GUID 表、依赖表。**已被 C# AssetManager 替代**。

### 4.4 NNRuntimeAssetLegacy

旧版资产类型实现（Audio、GalGame、LuaScript、Scene、Texture、UI、Video）。**已被 C# 资产系统替代**。

### 4.5 NNRuntimeRHI

原始 OpenGL 3 渲染硬件接口。**已被 NNRuntimeDiligent 替代**。

### 4.6 NNRuntimeRenderAssets

GPU 渲染资产管理。CPU 资产 → GPU 资源桥接。**已被 C# 资产管线替代**。

### 4.7 NNRuntimeImageCodec

stb_image 图像解码。静态库。

### 4.8 NNRuntimeManagedBridge / NNRuntimeManagedHostLegacy

原生→托管运行时桥接（CoreCLR hostfxr）。**已被 Entry.Bootstrap + 产品 hostfxr 替代**。

### 4.9 NNRuntimeMediaAssets

媒体资产烘焙管线。**存在编译错误，待修复**。

---

## 5. 依赖关系图

```
                        ┌─────────────────────────────────────────────┐
                        │              C# 托管代码                      │
                        └──────────────────┬──────────────────────────┘
                                           │ P/Invoke (C ABI)
                        ┌──────────────────▼──────────────────────────┐
                        │        NNRuntimeEngineServices              │
                        │        (ABI 转发层)                          │
                        └──┬───┬───┬───┬───┬───┬───┬──────────────────┘
                           │   │   │   │   │   │   │
              ┌────────────┘   │   │   │   │   │   └──────────────┐
              ▼                ▼   ▼   ▼   ▼   ▼                  ▼
    ┌─────────────┐  ┌────────┐ ┌───┐ ┌───┐ ┌───┐ ┌──────┐  ┌──────────┐
    │Application  │  │Engine  │ │VFS│ │Lua│ │Rml│ │Render│  │Renderer2D│
    │(SDL3 宿主)  │  │(调度器)│ │   │ │   │ │UI │ │ er2D │  │          │
    └──────┬──────┘  └───┬────┘ └─┬─┘ └─┬─┘ └─┬─┘ └──┬───┘  └────┬─────┘
           │             │        │     │      │       │           │
           ▼             ▼        ▼     ▼      ▼       ▼           ▼
    ┌──────────────────────────────────────────────────────────────────┐
    │                        NNRuntimeCore                             │
    │              (接口、事件、输入、视口、数据)                          │
    └──────────────────────────────┬───────────────────────────────────┘
                                   │
    ┌──────────────────────────────▼───────────────────────────────────┐
    │                        NNRuntimeImGui                            │
    │              (ImGui 核心 + 扩展 + Diligent 后端)                   │
    └──────────────────────────────┬───────────────────────────────────┘
                                   │
    ┌──────────────────────────────▼───────────────────────────────────┐
    │                     NNRuntimeDiligent                             │
    │              (Diligent Engine COM 绑定)                           │
    └──────────────────────────────┬───────────────────────────────────┘
                                   │
    ┌──────────────────────────────▼───────────────────────────────────┐
    │                        NNRuntimePak                              │
    │              (.pak 打包/解包 + VFS 挂载)                           │
    └──────────────────────────────┬───────────────────────────────────┘
                                   │
    ┌──────────────────────────────▼───────────────────────────────────┐
    │                       NNRuntimeMedia                             │
    │              (音频/视频播放 + GPU 纹理)                            │
    └──────────────────────────────┬───────────────────────────────────┘
                                   │
    ┌──────────────────────────────▼───────────────────────────────────┐
    │                         NNAPI                                    │
    │              (ABI 契约 + Stub，零依赖)                             │
    └──────────────────────────────────────────────────────────────────┘
```

---

## 6. ABI 层架构

### 设计原则

1. **所有跨 DLL 边界的调用通过 `extern "C"` 函数指针表**，不使用 C++ vtable
2. **Handle 化**：所有对象引用使用 `uint64_t` opaque handle
3. **POD 结构体**：跨边界传递的数据必须是 POD（Plain Old Data）
4. **调用约定**：统一使用 `NN_ENGINE_ABI_STDCALL`（Windows: `__stdcall`）
5. **内存管理**：输出缓冲区用 `malloc` 分配，调用方用 `freeBuffer` 释放，避免 CRT 不匹配

### 调用链

```
C# 代码
  ↓ P/Invoke
NNNativeAPI (函数指针表)
  ↓ 函数指针调用
NNRuntimeEngineServices (转发层)
  ↓ 直接调用
NNRuntimeApplication / NNRuntimeEngine / NNRuntimeVFS / ... (真实实现)
```

### API 版本管理

- `NNNativeAPI.apiVersion`：顶层版本号，当前 **3**
- `NNNativeEngineAPI` 布局版本：当前 **32**
- 版本不匹配时 C# 端应拒绝加载

---

## 7. 构建系统

### CMake 配置

- **C++ 标准**：C++17
- **构建工具**：CMake 3.x
- **编译器标志**：`/MP`（MSVC 并行编译）、`/utf-8`（UTF-8 源/执行字符集）

### 模块类型

| 类型 | 说明 |
|------|------|
| STATIC | 编译进最终 DLL，不单独分发（NNAPI、NNRuntimeEngine、NNRuntimeApplication） |
| SHARED | 独立 DLL，有导出宏（NNRuntimeCore、NNRuntimeVFS、NNRuntimeLua 等） |

### 导出宏约定

每个 SHARED 模块有自己的导出宏：

| 模块 | 导出宏 |
|------|--------|
| NNRuntimeCore | `NN_RUNTIME_CORE_API` |
| NNRuntimeVFS | `NN_RUNTIME_VFS_API` |
| NNRuntimeApplication | `NN_RUNTIME_APPLICATION_API` |
| NNRuntimeLua | `NNRUNTIMELUA_EXPORT` |

### 活跃构建目标（CMakeLists.txt）

```cmake
add_subdirectory("NNRuntimeLua")
add_subdirectory("NNRuntimeVFS")
add_subdirectory("NNRuntimePak")
add_subdirectory("NNRuntimeImGui")
add_subdirectory("NNRuntimeCore")
add_subdirectory("NNRuntimeApplication")
add_subdirectory("NNAPI")
add_subdirectory("NNRuntimeEngine")
add_subdirectory("NNRuntimeRenderer2D")
add_subdirectory("NNRuntimeEngineServices")
add_subdirectory("NNRuntimeRmlui")
add_subdirectory("NNRuntimeMedia")
```

---

## 8. 当前状态与进度

### 已完成

- [x] NNAPI ABI 契约层（Layout Version 32, API Version 3）
- [x] NNRuntimeCore 核心框架（接口、事件、输入、视口）
- [x] NNRuntimeEngine 引擎调度器（5 Tick Group + 固定步长）
- [x] NNRuntimeApplication 应用宿主（SDL3 + 事件队列 + ImGui 层）
- [x] NNRuntimeVFS 虚拟文件系统（Native/Memory/Zip 三后端）
- [x] NNRuntimeLua Lua 5.4.8 VM
- [x] NNRuntimeImGui ImGui 1.92.3 + 扩展集
- [x] NNRuntimeRenderer2D 2D 精灵批量渲染
- [x] NNRuntimeRmlui RmlUI 集成（Diligent 后端）
- [x] NNRuntimePak Pak 打包系统
- [x] NNRuntimeEngineServices ABI 转发层
- [x] C# AssetManager 全面接管（C++/C# Asset ABI 已删除）
- [x] C# Friflo ECS 替代 NNRuntimeScene

### 进行中

- [ ] NNRuntimeMedia 音视频播放（部分源文件 TODO）
- [ ] ImGui OpenGL3 → Diligent 后端迁移
- [ ] Viewport RenderCommands 修复验证

### 待做

- [ ] NNRuntimeMediaAssets 编译错误修复
- [ ] Path 类型迁移 Phase 5
- [ ] Legacy 模块清理（确认无依赖后删除）

---

## 附录：模块文件统计

| 模块 | 头文件 | 源文件 | 总计 |
|------|--------|--------|------|
| NNAPI | ~15 | ~12 | ~27 |
| NNRuntimeCore | 27 | 17 | 44 |
| NNRuntimeEngine | 12 | 6 | 18 |
| NNRuntimeApplication | 13 | 10 | 23 |
| NNRuntimeVFS | 21 | 4 | 25 |
| NNRuntimeLua | ~10 | ~20 | ~30 |
| NNRuntimeImGui | ~30 | ~15 | ~45 |
| NNRuntimeRenderer2D | ~5 | ~3 | ~8 |
| NNRuntimeRmlui | ~20 | ~8 | ~28 |
| NNRuntimePak | ~8 | ~4 | ~12 |
| NNRuntimeMedia | ~10 | ~6 | ~16 |
| NNRuntimeEngineServices | ~5 | ~10 | ~15 |
| **合计** | **~176** | **~115** | **~291** |
