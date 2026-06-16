# NNRuntimeRenderer2D — 模块架构与进度文档

> 最后更新: 2026-06-14
> 模块状态: **已完成 Phase 0-1（基础渲染 + ECS 集成）**

---

## 目录

1. [模块概述](#1-模块概述)
2. [目录结构](#2-目录结构)
3. [构建配置](#3-构建配置)
4. [模块依赖关系](#4-模块依赖关系)
5. [核心类架构](#5-核心类架构)
6. [各类详细分析](#6-各类详细分析)
7. [渲染管线工作流程](#7-渲染管线工作流程)
8. [着色器系统](#8-着色器系统)
9. [公共 API 参考](#9-公共-api-参考)
10. [设计模式与架构特点](#10-设计模式与架构特点)
11. [已知问题与技术债务](#11-已知问题与技术债务)
12. [开发进度记录](#12-开发进度记录)
13. [未来规划](#13-未来规划)

---

## 1. 模块概述

### 1.1 模块定位

`NNRuntimeRenderer2D` 是 Neverness 引擎的 **2D 精灵渲染模块**，负责将 ECS 场景中的 2D 精灵实体渲染为离屏纹理，供编辑器视口 (EditorViewport)、RmlUI 叠加层或游戏画面使用。

### 1.2 核心职责

| 职责 | 说明 |
|------|------|
| ECS 数据收集 | 从 entt registry 中收集 Transform + SpriteRenderer 组件数据 |
| 纹理懒加载 | 首次遇到未加载纹理时同步加载资产并上传 GPU |
| 离屏渲染 | 通过 FramebufferObject 将场景渲染为 RGBA8 纹理 |
| 纹理句柄输出 | 返回 `uint64_t` 纹理句柄供 ImGui.Image / C# EditorViewport 使用 |
| 相机管理 | 从 ECS 查询主相机，计算 ViewProjection 矩阵 |

### 1.3 设计目标

- **零 Diligent 头文件泄漏**: 公共头文件不包含任何 Diligent 类型
- **一行渲染**: `SceneRenderer::Render(scene, w, h)` 完成全部工作
- **ECS 数据驱动**: 渲染参数全部来自 ECS 组件，无硬编码
- **接口抽象 + 后端穿透**: 资源创建走 `INNRenderDevice` 接口，渲染循环直接用 raw Diligent context

---

## 2. 目录结构

```
NNRuntimeRenderer2D/
├── CMakeLists.txt                          # 构建配置
├── Docs/                                   # 文档目录
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md # 本文档
├── Include/Renderer2D/                     # 公共头文件
│   ├── BuiltinShaders.h                   # 内置 HLSL 着色器源码
│   ├── CameraData.h                       # 相机渲染数据 POD
│   ├── FramebufferObject.h                # 离屏渲染目标封装
│   ├── Renderer2D.h                       # 核心 2D 渲染器 (PIMPL)
│   ├── Renderer2DExport.h                 # DLL 导出/导入宏
│   ├── SceneRenderer.h                    # 场景渲染顶层入口
│   ├── SpriteDrawCommand.h                # Sprite 绘制命令 POD
│   └── SpriteRenderSystem.h               # ECS Sprite 命令收集器
└── Source/                                 # 实现文件
    ├── FramebufferObject.cpp              # FBO 创建/销毁/Resize
    ├── Renderer2D.cpp                     # 核心渲染逻辑 + PIMPL Impl
    ├── SceneRenderer.cpp                  # 管线串联 + 相机查找
    └── SpriteRenderSystem.cpp             # ECS 遍历 + 纹理懒加载
```

**文件统计**: 8 个头文件、4 个源文件、1 个 CMakeLists.txt

---

## 3. 构建配置

### 3.1 CMake 目标

```cmake
add_library(NevernessRuntime-Renderer2D SHARED ${SOURCES})
```

| 属性 | 值 |
|------|-----|
| 目标名 | `NevernessRuntime-Renderer2D` |
| 类型 | SHARED (DLL) |
| C++ 标准 | C++17 |
| 编译定义 | `NN_RUNTIME_RENDERER2D_EXPORT` |
| 字符编码 | MSVC: `/utf-8`, GCC/Clang: `-finput-charset=UTF-8` |

### 3.2 Include 路径 (PRIVATE)

模块的 include 路径全部为 PRIVATE，不暴露给外部消费者：

```
Engine/Source/Runtime                          # Scene/RenderAssets/Asset 模块
Engine/Source/Core                             # 基础数学类型
Engine/Source/Rendering/NNRuntimeRender        # 渲染抽象层接口
Engine/Source/Rendering/NNRuntimeCore          # 渲染核心类型
Engine/Source/Rendering/NNRuntimeDiligent      # Diligent 后端实现
Engine/Source/Rendering/NNRuntimeDiligent/Include  # Diligent 内部头文件
Engine/Source/ThirdParty/DiligentEngine/DiligentCore  # Diligent 原始头文件
```

---

## 4. 模块依赖关系

### 4.1 编译时依赖 (CMake PRIVATE 链接)

```
NNRuntimeRenderer2D
├── NevernessRuntime-Scene        (ECS 查询: entt registry, 组件类型)
├── NevernessRuntime-RenderAssets  (纹理缓存: GUID → GPU 资源映射)
├── NevernessRuntime-Asset         (资产加载: LoadAssetSync)
├── NNRuntimeRender                (渲染抽象层: INNRenderDevice 等接口)
├── NNRuntimeDiligent              (Diligent 后端: NNDiligentDevice 等实现)
├── NevernessCore-Core             (基础数学: matrix, vector, glm)
└── DiligentEngine (隐式)          (GPU API: IRenderDevice, IDeviceContext 等)
```

### 4.2 详细依赖用途

| 模块 | 接口使用 | 具体类/函数 |
|------|----------|-------------|
| **NNRuntimeScene** | ECS 查询 | `NNRuntimeScene.GetRegistry()`, entt `registry.view<>()` |
| **NNRuntimeScene** | 组件数据 | `NNTransformComponent.WorldMatrix`, `NNSpriteRendererComponent.*`, `NNCameraComponent.*` |
| **NNRuntimeRender** | 渲染抽象层 | `INNRenderDevice`, `INNBuffer`, `INNTexture`, `INNSampler`, `INNPipelineState`, `INNShader`, `INNRenderTarget` |
| **NNRuntimeDiligent** | Diligent 后端 | `NNDiligentDevice.GetDiligentDevice/Context()`, `NNDiligentBuffer.GetDiligentBuffer()`, `NNDiligentTexture.GetDiligentTexture()`, `NNDiligentSampler.GetDiligentSampler()`, `NNDiligentRenderTarget.GetColorSRV/View/DepthView()` |
| **NNRuntimeRenderAssets** | 纹理缓存 | `NNRenderAssetManager.GetCacheKeyByGuidLow()`, `LoadTextureFromBlob()`, `GetImGuiTextureHandle()` |
| **NNRuntimeAsset** | 资产加载 | `NNAssetManager.LoadAssetSync()`, `GetBlobByType()` |
| **NevernessCore-Core** | 数学 + 日志 | `glm::inverse()`, `matrix`, `H_LOG_*` |

### 4.3 运行时依赖

```
C# EditorViewport
    ↓ (通过 NNNativeEngineAPI)
SceneRenderer::Render()
    ↓ (返回 uint64_t 纹理句柄)
ImGui.Image / RmlUI Overlay
```

- C# EditorViewport 通过 `NNNativeEngineAPI` 调用 `SceneRenderer::Render()`
- 返回的 `uint64_t` 纹理句柄传给 ImGui.Image 显示
- `FramebufferObject` 也可供 RmlUI 叠加渲染使用 (`GetFramebufferObject()`)

### 4.4 依赖关系图

```
┌─────────────────────────────────────────────────────────────────┐
│                    C# EditorViewport / Game Loop                │
└──────────────────────────────┬──────────────────────────────────┘
                               │ NNNativeEngineAPI (uint64_t handle)
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                  NNRuntimeRenderer2D (本模块)                    │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────────┐  │
│  │SceneRenderer │  │ Renderer2D   │  │ SpriteRenderSystem    │  │
│  │ (顶层入口)    │──│ (GPU 渲染)   │  │ (ECS 数据收集)        │  │
│  └──────┬───────┘  └──────┬───────┘  └───────────┬───────────┘  │
│         │                 │                       │              │
│  ┌──────┴───────┐  ┌──────┴───────┐  ┌───────────┴───────────┐  │
│  │FramebufferObj│  │ BuiltinShaders│  │ SpriteDrawCommand     │  │
│  │ (离屏 RT)    │  │ (HLSL 源码)  │  │ CameraData (POD)     │  │
│  └──────────────┘  └──────────────┘  └───────────────────────┘  │
└──────────────────────────────┬──────────────────────────────────┘
                               │
        ┌──────────┬───────────┼───────────┬──────────┐
        ▼          ▼           ▼           ▼          ▼
   NNRuntime  NNRuntime   NNRuntime   NNRuntime   NevernessCore
    Scene     RenderAssets  Render    Diligent      Core
   (ECS)     (纹理缓存)   (抽象层)   (Diligent)   (数学)
```

---

## 5. 核心类架构

### 5.1 组合模式 (无继承)

本模块不使用继承，全部采用**组合模式**。核心类关系：

```
SceneRenderer (顶层入口，对外 API)
├── Renderer2D          (实际 GPU 渲染)
│   └── Impl            (PIMPL，持有所有 Diligent 资源)
├── SpriteRenderSystem  (ECS 数据收集)
├── FramebufferObject   (离屏 RT 封装)
└── CameraData          (POD，从 ECS 计算)
```

### 5.2 数据流

```
ECS Scene
    │
    ├─→ SpriteRenderSystem.Collect() ──→ vector<SpriteDrawCommand>
    │                                       │
    └─→ SceneRenderer.FindMainCamera() ──→ CameraData
                                            │
                                            ▼
                        Renderer2D.SetRenderTarget(FBO.RTV, FBO.DSV)
                        Renderer2D.BeginScene(camera, w, h)
                        Renderer2D.Submit(commands)
                        Renderer2D.EndScene()
                                            │
                                            ▼
                        FramebufferObject.GetColorTextureHandle()
                                            │
                                            ▼
                              uint64_t (供 ImGui.Image / C# EditorViewport)
```

### 5.3 职责分离

| 类 | 职责 | 数据 | 副作用 |
|----|------|------|--------|
| `SceneRenderer` | 管线串联、相机查找 | 持有 Renderer2D + SpriteRenderSystem + FBO | 无 |
| `Renderer2D` | GPU 渲染 | PIMPL 持有所有 Diligent 资源 | 无 |
| `SpriteRenderSystem` | ECS 数据收集 | 无状态 | 修改 `sprite.TextureRuntimeId` (懒加载) |
| `FramebufferObject` | 离屏 RT 封装 | 持有 INNRenderTarget | 无 |
| `SpriteDrawCommand` | 绘制命令 POD | 纯数据 | 无 |
| `CameraData` | 相机数据 POD | 纯数据 | 无 |

---

## 6. 各类详细分析

### 6.1 SpriteDrawCommand (POD 结构)

**文件**: `Include/Renderer2D/SpriteDrawCommand.h`

纯数据结构，零逻辑，内存布局友好：

```cpp
struct SpriteDrawCommand {
    float         Transform[16];   // 4x4 世界矩阵 (列主序)
    uint64_t      TextureHandle;   // RHI 纹理句柄 (0 = 白色默认)
    float         Color[4];        // RGBA tint [0,1]
    float         UvRect[4];       // [u0, v0, u1, v1] Atlas UV
    uint32_t      Layer;           // 渲染层 (排序用)
    uint32_t      SortOrder;       // 层内排序 (大的后渲染)
    BlendMode     Blend;           // 混合模式枚举
    bool          FlipX;           // 水平翻转
    bool          FlipY;           // 垂直翻转
};
```

**BlendMode 枚举**:

| 值 | 名称 | 说明 |
|----|------|------|
| 0 | `Alpha` | 标准 Alpha 混合 (SrcAlpha, InvSrcAlpha) |
| 1 | `Additive` | 叠加混合 |
| 2 | `Multiply` | 正片叠底 |
| 3 | `Opaque` | 不透明 |
| 4 | `Premultiplied` | 预乘 Alpha |

> **注意**: BlendMode 字段已定义但当前未使用，Renderer2D 固定使用 Alpha 混合。

### 6.2 CameraData (POD 结构)

**文件**: `Include/Renderer2D/CameraData.h`

```cpp
struct CameraData {
    float ViewMatrix[16];           // 逆 Transform WorldMatrix
    float ProjectionMatrix[16];     // 来自 CameraComponent
    float ViewProjectionMatrix[16]; // Proj * View
    float OrthoWidth  = 10.0f;      // 正交宽度
    float OrthoHeight = 10.0f;      // 正交高度
    float Near        = -1.0f;      // 近平面
    float Far         =  1.0f;      // 远平面
};
```

### 6.3 SpriteRenderSystem (ECS 数据收集器)

**文件**: `Include/Renderer2D/SpriteRenderSystem.h`, `Source/SpriteRenderSystem.cpp`

**职责**: 遍历 entt registry 中同时拥有 `NNTransformComponent` + `NNSpriteRendererComponent` 的实体，生成 `SpriteDrawCommand` 数组。

**工作流程**:

```
1. registry.view<NNTransformComponent, NNSpriteRendererComponent>()
   │
   ├─ 跳过不可见 (检查 NNSpriteFlags::Visible)
   │
   ├─ 纹理懒加载 (如果 TextureRuntimeId == 0 且 TextureAsset.low != 0):
   │   ├─ NNRenderAssetManager.GetCacheKeyByGuidLow() → 查缓存
   │   ├─ 缓存未命中 → NNAssetManager.LoadAssetSync() → 同步加载
   │   ├─ NNRenderAssetManager.LoadTextureFromBlob() → 上传 GPU
   │   └─ sprite.TextureRuntimeId = cacheKey (uint32_t 截断)
   │
   ├─ NNRenderAssetManager.GetImGuiTextureHandle(cacheKey) → uint64_t
   │
   └─ 构建 SpriteDrawCommand (Transform, Color, UvRect, Layer, SortOrder, Flip)
   │
   ▼
2. 排序: Layer 升序 → SortOrder 升序
```

**重要副作用**: 该系统直接修改了 ECS 组件数据 (`sprite.TextureRuntimeId`)，这是副作用性的懒初始化模式。

### 6.4 FramebufferObject (离屏渲染目标)

**文件**: `Include/Renderer2D/FramebufferObject.h`, `Source/FramebufferObject.cpp`

**职责**: 封装 Diligent `INNRenderTarget`，对外只暴露 `uint64_t` 纹理句柄，不泄露 Diligent 类型。

**接口**:

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `Initialize(device, w, h)` | `bool` | 创建 RGBA8_UNORM 颜色 + D24_S8 深度 |
| `Shutdown()` | `void` | 释放所有资源 |
| `Resize(w, h)` | `void` | Shutdown + 重新 Initialize |
| `GetColorTextureHandle()` | `uint64_t` | `reinterpret_cast<ITextureView* SRV>` 供 ImGui |
| `GetColorRTV()` | `void*` | Diligent RTV，供 Renderer2D BeginRenderPass |
| `GetDepthDSV()` | `void*` | Diligent DSV，供 Renderer2D BeginRenderPass |
| `GetRenderTarget()` | `INNRenderTarget*` | 内部渲染目标指针 |

**所有权模型**: `m_RenderTarget` 持有所有权 (手动 Release)，`m_Device` 是观察指针 (不持有)。

**纹理格式**:
- 颜色: `RGBA8_UNORM` (8 位每通道，归一化)
- 深度: `D24_UNORM_S8_UINT` (24 位深度 + 8 位模板)

### 6.5 Renderer2D (核心渲染器)

**文件**: `Include/Renderer2D/Renderer2D.h`, `Source/Renderer2D.cpp`

**职责**: 接收 `SpriteDrawCommand` 数组，通过 Diligent 后端批量绘制 Quad。

#### 6.5.1 PIMPL 模式

`Renderer2D` 使用 `Impl` 结构体隐藏所有 Diligent 实现细节。公共头文件不暴露任何 Diligent 类型。

**Impl 内部持有**:

| 类别 | 资源 | 持有方式 |
|------|------|----------|
| Diligent 原始指针 | `IRenderDevice*`, `IDeviceContext*` | 从 NNDiligentDevice 提取，观察指针 |
| NN 资源 | VB, IB, CB, WhiteTexture, Sampler | `NNRef` 引用计数 |
| Diligent 缓存指针 | PSO, RenderPass, SRV, Sampler, CB, VB, IB | 从 NN 包装器提取，避免每帧 cast |
| Framebuffer | `IFramebuffer*`, RTV/DSV 指针, 附件数组 | Diligent 引用计数 |
| SRB 缓存 | `unordered_map<void*, RefCntAutoPtr<IShaderResourceBinding>>` | 按纹理 SRV 指针缓存 |
| 统计 | DrawCallCount, QuadCount | 计数器 |

#### 6.5.2 常量缓冲区布局

```cpp
struct SpriteCB {
    float ViewProjection[16]; // 64 bytes — 相机 VP 矩阵
    float Transform[16];      // 64 bytes — 世界变换矩阵
    float UvRect[4];          // 16 bytes — [u0, v0, u1, v1]
    float Color[4];           // 16 bytes — RGBA tint
    int   FlipX;              //  4 bytes — 水平翻转标志
    int   FlipY;              //  4 bytes — 垂直翻转标志
    int   Padding[2];         //  8 bytes — 对齐到 16 字节边界
};
// 总计: 176 字节，与 HLSL cbuffer 对齐
```

#### 6.5.3 Unit Quad 几何体

```
顶点布局:
  Position (float3) + TexCoord (float2)

顶点数据:
  (-0.5, -0.5, 0) → UV (0, 0)   // 左下
  ( 0.5, -0.5, 0) → UV (1, 0)   // 右下
  ( 0.5,  0.5, 0) → UV (1, 1)   // 右上
  (-0.5,  0.5, 0) → UV (0, 1)   // 左上

索引数据:
  {0, 1, 2, 2, 3, 0}  // 两个三角形
```

#### 6.5.4 Initialize 流程 (9 步)

```
1. 创建 HLSL 着色器 (VS + PS，通过 INNRenderDevice::CreateShader)
   │
2. 创建 Diligent RenderPass
   ├─ 颜色附件: RGBA8_UNORM, STORE_ACTION_STORE
   └─ 深度附件: D24_UNORM_S8_UINT, STORE_ACTION_DISCARD
   │
3. 创建 PSO (Pipeline State Object)
   ├─ PipelineType: GRAPHICS
   ├─ 顶点布局: Position(float3, offset 0) + TexCoord(float2, offset 12)
   ├─ 光栅化: CULL_MODE_NONE (2D 无需背面剔除)
   ├─ 混合: 标准 Alpha 混合 (SrcAlpha, InvSrcAlpha)
   ├─ 深度: 禁用 (2D)
   └─ 资源变量: u_Texture (MUTABLE) + u_Sampler (ImmutableSampler, LinearClamp)
   │
4. 创建 Static 顶点缓冲 (Unit Quad)
5. 创建 Static 索引缓冲 (6 indices)
6. 创建 Dynamic 常量缓冲 (每帧 Map/Unmap)
7. 创建 1x1 白色默认纹理 (0xFFFFFFFF)
8. 创建 Nearest/Clamp 采样器
9. VB/IB 状态转换 (COPY_DEST → VERTEX/INDEX_BUFFER)
```

#### 6.5.5 Submit 渲染流程 (每帧)

```
1. 设置 PSO、VB、IB (整个 Submit 共享)
   │
2. 对每个 SpriteDrawCommand:
   ├─ Map 常量缓冲 → 写入 VP + Transform + UvRect + Color + Flip
   ├─ 获取纹理 SRV (handle=0 → 用白色纹理)
   ├─ 查找/创建 SRB (按 SRV 指针缓存，首次创建时绑定 CB + Texture)
   ├─ CommitShaderResources
   └─ DrawIndexed (6 indices = 1 quad)
   │
3. 递增 DrawCallCount 和 QuadCount
```

#### 6.5.6 SRB 缓存策略

使用 `unordered_map<void*, SRB>` 按纹理 SRV 指针缓存 Shader Resource Binding。

- 相同纹理的多个 Sprite 共享同一个 SRB
- 只更新 CB (通过 Map/Unmap)
- 缓存粒度: per-texture

### 6.6 BuiltinShaders (内置着色器)

**文件**: `Include/Renderer2D/BuiltinShaders.h`

提供两个 `inline constexpr const char*`:

| 着色器 | 类型 | 说明 |
|--------|------|------|
| `SpriteVS` | 顶点着色器 | MVP 变换 + UV Rect 映射 + Flip 翻转 |
| `SpriteFS` | 片段着色器 | 纹理采样 × 颜色 tint |

> **注意**: 注释说 "硬编码 HLSL"，实际上 NNRuntimeRender 的 CreateShader 目前只支持 HLSL，所以从 GLSL 转换到了 HLSL。

### 6.7 SceneRenderer (顶层入口)

**文件**: `Include/Renderer2D/SceneRenderer.h`, `Source/SceneRenderer.cpp`

**职责**: 串联 ECS → SpriteRenderSystem → Renderer2D → Framebuffer 的完整渲染管线。

**对外核心 API**:

```cpp
// 一行完成: 查相机 → 收集命令 → 渲染 → 返回纹理句柄
uint64_t Render(Scene::NNRuntimeScene& scene, uint32_t width, uint32_t height);
```

#### 6.7.1 Render 流程

```
1. FramebufferObject::Resize (if needed)
   │
2. FindMainCamera(scene) → CameraData
   ├─ 查询第一个带 CameraComponent + TransformComponent 的实体
   ├─ 计算 ViewMatrix = inverse(WorldMatrix)
   ├─ 从 CameraComponent 获取 ProjectionMatrix
   ├─ 计算 ViewProjectionMatrix = Proj × View
   └─ 相机 Z 修正 (见下文)
   │
3. SpriteRenderSystem::Collect(scene, commands)
   │
4. Renderer2D::SetRenderTarget(FBO.RTV, FBO.DSV)
5. Renderer2D::BeginScene(camera, w, h)
6. Renderer2D::Submit(commands)
7. Renderer2D::EndScene()
   │
8. Return FBO.GetColorTextureHandle() → uint64_t
```

#### 6.7.2 相机 Z 修正

**问题**: 相机默认在 z=0，但 ortho 默认 near=0.3。右手坐标系相机看向 -Z，z=0 的 Sprite 在相机位置 (z_view=0)，位于近平面 z_view=-0.3 之后，NDC z=-1.0006 超出 [-1,1] 被裁剪。

**修复**: 将相机 Z 移到 `+(near+far)/2`，使 z=0 的 Sprite 落在 near/far 中间。

---

## 7. 渲染管线工作流程

### 7.1 完整管线图

```
┌─────────────────────────────────────────────────────────────────┐
│  C# EditorViewport / Game Loop                                  │
│  调用 SceneRenderer.Render(scene, width, height)                │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────────┐
│  SceneRenderer::Render                                            │
│                                                                  │
│  1. FramebufferObject::Resize (if needed)                         │
│     └─ 确保 FBO 尺寸匹配视口                                      │
│                                                                  │
│  2. FindMainCamera → CameraData                                   │
│     ├─ ECS 查询: CameraComponent + TransformComponent             │
│     ├─ 计算 ViewMatrix = inverse(WorldMatrix)                     │
│     ├─ 获取 ProjectionMatrix                                      │
│     ├─ 计算 VP = Proj × View                                     │
│     └─ 相机 Z 修正: Z = +(near+far)/2                            │
│                                                                  │
│  3. SpriteRenderSystem::Collect                                   │
│     ├─ Query ECS: Transform + SpriteRenderer                      │
│     ├─ 跳过不可见 (NNSpriteFlags::Visible)                        │
│     ├─ 懒加载纹理 (GUID → Asset → GPU upload → cache key)        │
│     ├─ Build SpriteDrawCommand[]                                  │
│     └─ Sort by Layer, then SortOrder                              │
│                                                                  │
│  4. Renderer2D::SetRenderTarget (RTV + DSV → Diligent FBO)       │
│                                                                  │
│  5. Renderer2D::BeginScene                                        │
│     ├─ Reset stats (DrawCallCount, QuadCount)                     │
│     ├─ Cache VP matrix                                            │
│     ├─ Set viewport                                               │
│     └─ BeginRenderPass (clear color=transparent black, depth=1.0) │
│                                                                  │
│  6. Renderer2D::Submit(commands)                                  │
│     ├─ Set PSO, VB, IB (shared across all sprites)                │
│     └─ For each command:                                          │
│         ├─ Map CB → write VP + Transform + UvRect + Color + Flip  │
│         ├─ Lookup/Create SRB by texture SRV                       │
│         ├─ CommitShaderResources                                   │
│         └─ DrawIndexed (6 indices = 1 quad)                       │
│                                                                  │
│  7. Renderer2D::EndScene → EndRenderPass                          │
│                                                                  │
│  8. Return FBO.GetColorTextureHandle() → uint64_t                 │
└──────────────────────────────────────────────────────────────────┘
```

### 7.2 每帧数据流

```
ECS Registry
    │
    ├─ NNTransformComponent.WorldMatrix ──→ SpriteDrawCommand.Transform
    ├─ NNSpriteRendererComponent.Color ───→ SpriteDrawCommand.Color
    ├─ NNSpriteRendererComponent.UvRect ──→ SpriteDrawCommand.UvRect
    ├─ NNSpriteRendererComponent.Layer ───→ SpriteDrawCommand.Layer
    ├─ NNSpriteRendererComponent.SortOrder → SpriteDrawCommand.SortOrder
    ├─ NNSpriteRendererComponent.FlipX/Y ──→ SpriteDrawCommand.FlipX/Y
    └─ NNSpriteRendererComponent.TextureAsset → 纹理懒加载 → TextureHandle
                                                          │
                                                          ▼
                                               SpriteDrawCommand.TextureHandle
                                                          │
                                                          ▼
                                               Renderer2D::Submit()
                                                          │
                                                          ▼
                                               GPU DrawIndexed (1 quad per sprite)
```

---

## 8. 着色器系统

### 8.1 顶点着色器 (SpriteVS)

```hlsl
cbuffer SpriteCB : register(b0) {
    float4x4 ViewProjection;  // 相机 VP 矩阵
    float4x4 Transform;       // 世界变换矩阵
    float4   UvRect;          // [u0, v0, u1, v1]
    float4   Color;           // RGBA tint
    int      FlipX;           // 水平翻转
    int      FlipY;           // 垂直翻转
    int2     Padding;         // 对齐
};

struct VSInput {
    float3 Position : ATTRIB0;
    float2 TexCoord : ATTRIB1;
};

struct PSInput {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

PSInput main(VSInput input) {
    PSInput output;
    output.Position = mul(ViewProjection, mul(Transform, float4(input.Position, 1.0)));

    // UV 从 Rect 映射
    float2 uv = input.TexCoord;
    if (FlipX) uv.x = 1.0 - uv.x;
    if (FlipY) uv.y = 1.0 - uv.y;
    output.TexCoord = lerp(UvRect.xy, UvRect.zw, uv);

    output.Color = Color;
    return output;
}
```

### 8.2 片段着色器 (SpriteFS)

```hlsl
Texture2D    u_Texture : register(t0);
SamplerState u_Sampler : register(s0);

struct PSInput {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 texColor = u_Texture.Sample(u_Sampler, input.TexCoord);
    return texColor * input.Color;
}
```

### 8.3 着色器特点

- **硬编码 HLSL**: 源码直接嵌入 C++ 头文件，运行时编译
- **UV Rect 映射**: 支持 Atlas 纹理，通过 `lerp` 从 Rect 范围映射 UV
- **Flip 翻转**: 支持水平/垂直翻转，通过整数标志控制
- **颜色 tint**: 片段着色器将纹理颜色与顶点颜色相乘

---

## 9. 公共 API 参考

### 9.1 SceneRenderer (推荐的顶层入口)

```cpp
class NN_RUNTIME_RENDERER2D_API SceneRenderer {
public:
    // 初始化整个 2D 渲染管线
    bool Initialize(INNRenderDevice* device);

    // 清理所有资源
    void Shutdown();

    // 一行完成场景渲染，返回纹理句柄
    uint64_t Render(Scene::NNRuntimeScene& scene, uint32_t width, uint32_t height);

    // 获取上次渲染的纹理句柄
    uint64_t GetOutputTextureHandle() const;

    // 视口尺寸变更通知
    void OnViewportResize(uint32_t width, uint32_t height);

    // 统计信息
    uint32_t GetDrawCallCount() const;
    uint32_t GetQuadCount() const;

    // 获取 FBO (供 RmlUI 使用)
    FramebufferObject* GetFramebufferObject();
};
```

### 9.2 Renderer2D (底层渲染器)

```cpp
class NN_RUNTIME_RENDERER2D_API Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    // 初始化: 编译着色器、创建几何体和默认纹理
    bool Initialize(INNRenderDevice* device);

    // 清理
    void Shutdown();

    // 设置渲染目标
    void SetRenderTarget(void* rtv, void* dsv);

    // 开始帧
    void BeginScene(const CameraData& camera, uint32_t width, uint32_t height);

    // 提交绘制命令
    void Submit(std::vector<SpriteDrawCommand>& commands);

    // 结束帧
    void EndScene();

    // 统计
    uint32_t GetDrawCallCount() const;
    uint32_t GetQuadCount() const;
};
```

### 9.3 FramebufferObject (离屏 RT)

```cpp
class NN_RUNTIME_RENDERER2D_API FramebufferObject {
public:
    FramebufferObject();
    ~FramebufferObject();

    // 创建渲染目标
    bool Initialize(INNRenderDevice* device, uint32_t width, uint32_t height);

    // 清理
    void Shutdown();

    // 重设尺寸
    void Resize(uint32_t width, uint32_t height);

    // 获取纹理句柄 (供 ImGui)
    uint64_t GetColorTextureHandle() const;

    // 获取 Diligent 视图指针
    void* GetColorRTV() const;
    void* GetDepthDSV() const;

    // 获取内部渲染目标
    INNRenderTarget* GetRenderTarget() const;
};
```

### 9.4 SpriteRenderSystem (ECS 收集器)

```cpp
class NN_RUNTIME_RENDERER2D_API SpriteRenderSystem {
public:
    // 收集并排序 Sprite 绘制命令
    void Collect(Scene::NNRuntimeScene& scene, std::vector<SpriteDrawCommand>& outCommands);
};
```

---

## 10. 设计模式与架构特点

### 10.1 设计模式

| 模式 | 应用 | 说明 |
|------|------|------|
| **PIMPL** | `Renderer2D` | 隐藏所有 Diligent 实现细节，公共头文件零 Diligent 类型 |
| **POD 数据驱动** | `SpriteDrawCommand`, `CameraData` | 纯数据结构，ECS 收集与 GPU 渲染完全解耦 |
| **组合模式** | 所有类 | 无继承，全部通过组合实现 |
| **接口抽象** | `INNRenderDevice` | 资源创建通过抽象接口 |
| **后端穿透** | `IDeviceContext` | 渲染循环直接使用 raw Diligent context |
| **缓存** | SRB 缓存 | 按纹理 SRV 指针缓存 Shader Resource Binding |
| **懒初始化** | 纹理加载 | 首次遇到未加载纹理时同步加载 |

### 10.2 架构特点

1. **双层 API 设计**: 资源创建用 `INNRenderDevice` 接口 (抽象层)，但渲染循环直接用 raw Diligent `IDeviceContext`，原因是 `NNRuntimeRender` 缺少 SRB 绑定接口

2. **一行渲染**: `SceneRenderer::Render(scene, w, h)` 完成全部工作，返回 `uint64_t` 纹理句柄

3. **零 Diligent 泄漏**: 公共头文件不包含任何 Diligent 类型，使用者无需了解底层实现

4. **ECS 数据驱动**: 渲染参数全部来自 ECS 组件，无硬编码

5. **离屏渲染**: 所有渲染输出到 FBO，纹理句柄可复用于 ImGui/RmlUI/游戏画面

---

## 11. 已知问题与技术债务

### 11.1 性能问题

| 问题 | 严重度 | 位置 | 说明 |
|------|--------|------|------|
| **每个 Sprite 一个 DrawCall** | 中 | `Renderer2D::Submit()` | 当前实现没有做批处理合批 (batching)，每个 Sprite 独立 Map/Unmap CB + DrawIndexed |
| **纹理懒加载是阻塞的** | 高 | `SpriteRenderSystem::Collect()` | 调用 `LoadAssetSync()` 会阻塞当前帧，可能造成卡顿 |
| **SRB 缓存无驱逐** | 低 | `Renderer2D::Impl::m_SrbCache` | `unordered_map<void*, SRB>` 只增不减，长时间运行可能积累大量 SRB |

### 11.2 功能缺失

| 问题 | 严重度 | 说明 |
|------|--------|------|
| **BlendMode 未实现** | 低 | `SpriteDrawCommand` 中有 `BlendMode` 字段，但 `Renderer2D::Submit()` 中 PSO 是固定的 Alpha 混合，没有根据命令动态切换混合状态 |
| **无批处理合批** | 中 | 相同纹理的 Sprite 可以合并为一次 DrawCall，但当前每个 Sprite 独立绘制 |
| **无后处理** | - | 只有一个 RenderPass，没有后处理管线 |
| **无多相机支持** | - | `FindMainCamera` 只查询第一个相机实体 |

### 11.3 代码质量

| 问题 | 位置 | 说明 |
|------|------|------|
| **Diligent 后端穿透** | `Renderer2D::Impl` | 渲染循环直接使用 raw Diligent `IDeviceContext`，而不是通过 `NNRuntimeRender` 接口 |
| **副作用性懒初始化** | `SpriteRenderSystem::Collect()` | 直接修改 ECS 组件数据 (`sprite.TextureRuntimeId`) |
| **硬编码着色器** | `BuiltinShaders.h` | HLSL 源码嵌入 C++ 头文件，运行时编译 |

---

## 12. 开发进度记录

### Phase 0: 基础框架 ✅ 已完成

- [x] 模块目录结构建立
- [x] CMakeLists.txt 配置
- [x] DLL 导出宏定义
- [x] 基础 POD 结构 (SpriteDrawCommand, CameraData)

### Phase 1: 核心渲染 ✅ 已完成

- [x] FramebufferObject 实现 (RGBA8 + D24S8)
- [x] Renderer2D PIMPL 实现
- [x] 内置 HLSL 着色器 (SpriteVS + SpriteFS)
- [x] Unit Quad 几何体
- [x] 常量缓冲区布局
- [x] SRB 缓存机制
- [x] 白色默认纹理

### Phase 2: ECS 集成 ✅ 已完成

- [x] SpriteRenderSystem 实现
- [x] ECS 遍历 (Transform + SpriteRenderer)
- [x] 纹理懒加载 (GUID → Asset → GPU)
- [x] Layer + SortOrder 排序
- [x] SceneRenderer 顶层入口
- [x] 主相机查找 + Z 修正

### Phase 3: 优化 (待规划)

- [ ] 批处理合批 (相同纹理合并 DrawCall)
- [ ] 异步纹理加载 (替换 LoadAssetSync)
- [ ] SRB 缓存驱逐策略
- [ ] BlendMode 动态切换

### Phase 4: 扩展 (待规划)

- [ ] 多相机支持
- [ ] 后处理管线
- [ ] 粒子系统集成
- [ ] Tilemap 渲染

---

## 13. 未来规划

### 13.1 短期优化 (Phase 3)

1. **批处理合批**: 将相同纹理的 Sprite 合并为一次 DrawCall，减少 GPU 状态切换
2. **异步纹理加载**: 替换 `LoadAssetSync()` 为异步加载，避免阻塞主线程
3. **SRB 缓存驱逐**: 添加 LRU 或引用计数驱逐策略，防止内存泄漏
4. **BlendMode 实现**: 根据 `SpriteDrawCommand.Blend` 动态切换 PSO

### 13.2 中期扩展 (Phase 4)

1. **多相机支持**: 支持多个相机实体，渲染到不同 FBO
2. **后处理管线**: 添加后处理 Pass (模糊、色调映射等)
3. **粒子系统集成**: 与 ECS 粒子系统对接
4. **Tilemap 渲染**: 支持 Tilemap 组件的高效渲染

### 13.3 长期目标

1. **渲染管线抽象**: 将 2D/3D 渲染管线统一到通用框架
2. **着色器热重载**: 运行时重新编译着色器
3. **GPU 粒子**: 将粒子模拟移到 GPU
4. **多线程渲染**: 支持多线程命令录制

---

## 附录

### A. 文件清单

| 文件 | 类型 | 行数 (约) | 说明 |
|------|------|-----------|------|
| `CMakeLists.txt` | 构建 | 50 | CMake 配置 |
| `Include/Renderer2D/BuiltinShaders.h` | 头文件 | 100 | HLSL 着色器源码 |
| `Include/Renderer2D/CameraData.h` | 头文件 | 30 | 相机 POD |
| `Include/Renderer2D/FramebufferObject.h` | 头文件 | 50 | FBO 接口 |
| `Include/Renderer2D/Renderer2D.h` | 头文件 | 60 | 渲染器接口 |
| `Include/Renderer2D/Renderer2DExport.h` | 头文件 | 20 | DLL 宏 |
| `Include/Renderer2D/SceneRenderer.h` | 头文件 | 50 | 顶层入口接口 |
| `Include/Renderer2D/SpriteDrawCommand.h` | 头文件 | 40 | 绘制命令 POD |
| `Include/Renderer2D/SpriteRenderSystem.h` | 头文件 | 30 | ECS 收集器接口 |
| `Source/FramebufferObject.cpp` | 源文件 | 100 | FBO 实现 |
| `Source/Renderer2D.cpp` | 源文件 | 350 | 渲染器实现 + PIMPL |
| `Source/SceneRenderer.cpp` | 源文件 | 200 | 管线串联 + 相机查找 |
| `Source/SpriteRenderSystem.cpp` | 源文件 | 120 | ECS 遍历 + 纹理加载 |

### B. 关键常量

| 常量 | 值 | 说明 |
|------|-----|------|
| 颜色格式 | `RGBA8_UNORM` | FBO 颜色附件格式 |
| 深度格式 | `D24_UNORM_S8_UINT` | FBO 深度附件格式 |
| 默认清色 | `(0, 0, 0, 0)` | 透明黑色 |
| 默认深度 | `1.0` | 最远深度 |
| 默认正交宽度 | `10.0` | CameraData 默认值 |
| 默认近平面 | `-1.0` | CameraData 默认值 |
| 默认远平面 | `1.0` | CameraData 默认值 |
