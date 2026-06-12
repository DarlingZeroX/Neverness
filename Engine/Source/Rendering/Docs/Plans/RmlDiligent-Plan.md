# RmlDiligent — RmlUi + Diligent Engine 集成方案 v4

> **状态**: 📋 待审查
> **目录**: `Engine/Source/Experiments/RmlDiligent/`
> **目标**: 验证 RmlUi 通过 Diligent Engine 渲染的可行性，获得跨平台 GPU 后端能力
> **创建日期**: 2026-06-02
> **修订日期**: 2026-06-02（v4 最终定稿）

---

## 版本演进

| 版本 | 评分 | 关键变更 |
|------|------|---------|
| v1 | 80/100 | 初始设计，存在 7 个架构问题 |
| v2 | 88/100 | 修正全部 7 个问题 |
| v3 | 92/100 | 6 项精修 |
| v4 | — | P0 全部修正（见下方） |

### v4 修订清单

| # | 优先级 | 问题 | v3 方案 | v4 修正 |
|---|--------|------|---------|---------|
| 1 | P0 | RT Pool 生命周期漏洞 | 引用计数混用 | **PooledRenderTarget + custom deleter（析构自动归还 Pool）** |
| 2 | P0 | Stencil Ref 过于简化 | 固定值 1 | **m_StencilDepth 计数，嵌套递增** |
| 3 | P0 | Phase 0 只验证编译 | 编译通过即可 | **增加 Smoke Test（三角形渲染验证）+ PSO 创建验证** |
| 4 | P1 | Geometry 长期缓存假设 | 一 Geometry 一 Buffer | **标注仅实验阶段，正式版需 GeometryBufferAllocator** |
| 5 | P1 | SRB 缓存策略 | PSO×Texture 全局缓存 | **TextureHandle 内维护 ProgramId→SRB 懒创建 Map** |

---

## 0. 背景与动机

### 0.1 现状

Neverness 引擎当前 RmlUi 集成使用官方 Backend，直接绑定特定图形 API（DX12/Vulkan/GL）。
每个 Backend 都是一套独立实现，无法共享渲染逻辑。

### 0.2 目标

```
当前:
  RmlUi → RmlUi_Renderer_DX12 (直接调用 D3D12 API)
  RmlUi → RmlUi_Renderer_VK   (直接调用 Vulkan API)
  RmlUi → RmlUi_Renderer_GL3  (直接调用 OpenGL API)

目标:
  RmlUi → RmlDiligentRenderInterface → Diligent Engine
                                        ├── D3D11 后端
                                        ├── D3D12 后端
                                        ├── Vulkan 后端
                                        ├── OpenGL 后端
                                        └── Metal 后端
```

**核心价值**: 一次实现，自动获得所有 GPU 后端支持。

### 0.3 非目标

- ❌ 不替换官方 Backend（实验性质验证）
- ❌ 不暴露 Diligent 对象给 RmlUi
- ❌ 不直接包装 DX12 API（抽象 DX12 Backend 的设计思想，映射到 Diligent）

---

## 1. RmlUi RenderInterface API 分析

### 1.1 接口分类

RmlUi 的 `Rml::RenderInterface` 定义了以下接口，分为**必需**和**可选**两类：

#### 必需接口（纯虚函数）

| 接口 | 功能 | 复杂度 |
|------|------|--------|
| `CompileGeometry` | 编译顶点/索引数据为 GPU 几何体 | 中 |
| `RenderGeometry` | 渲染已编译的几何体 | 高 |
| `ReleaseGeometry` | 释放几何体资源 | 低 |
| `LoadTexture` | 从文件加载纹理 | 中 |
| `GenerateTexture` | 从像素数据生成纹理 | 中 |
| `ReleaseTexture` | 释放纹理资源 | 低 |
| `EnableScissorRegion` | 启用/禁用裁剪区域 | 低 |
| `SetScissorRegion` | 设置裁剪矩形 | 低 |

#### 可选接口（虚函数，有默认空实现）

| 接口 | 功能 | 复杂度 |
|------|------|--------|
| `SetTransform` | 设置变换矩阵 | 低 |
| `EnableClipMask` | 启用模板遮罩 | 中 |
| `RenderToClipMask` | 渲染遮罩几何体到模板缓冲 | 高 |
| `PushLayer` | 推入渲染层 | 高 |
| `CompositeLayers` | 合成渲染层（含 Filter） | 高 |
| `PopLayer` | 弹出渲染层 | 中 |
| `SaveLayerAsTexture` | 保存层为纹理 | 中 |
| `SaveLayerAsMaskImage` | 保存层为遮罩纹理 | 中 |
| `CompileFilter` | 编译滤镜（Blur/Shadow/ColorMatrix） | 高 |
| `ReleaseFilter` | 释放滤镜资源 | 低 |
| `CompileShader` | 编译自定义 Shader | 高 |
| `RenderShader` | 使用自定义 Shader 渲染 | 高 |
| `ReleaseShader` | 释放 Shader 资源 | 低 |

### 1.2 顶点格式

```cpp
struct Vertex {
    Vector2f position;   // 2D 位置
    ColourbPremultiplied color;  // 预乘 alpha 颜色（RGBA, uint8_t x 4）
    Vector2f tex_coord;  // 纹理坐标
};
// 总大小: 2*4 + 4 + 2*4 = 20 bytes
```

### 1.3 DX12 Backend 设计思想提炼

DX12 Backend 的核心设计模式（需要抽象，而非照搬）：

1. **Pipeline 变体系统**: 23 种 PSO 变体，通过 `ProgramId` 枚举选择
   - 设计思想: 状态组合（Texture/Color × Stencil 状态 × Blend 模式）
   - Diligent 映射: 预创建 `IPipelineState` 数组，运行时按状态索引

2. **常量缓冲区策略**: 每个 Draw Call 上传 transform + translate
   - 设计思想: 小数据频繁更新，使用动态 CB
   - Diligent 映射: `USAGE_DYNAMIC` Buffer + `Map`/`Unmap`

3. **子分配内存管理**: BufferMemoryManager 使用 VirtualBlock 子分配大块 GPU 内存
   - 设计思想: 减少小 Buffer 分配开销
   - Diligent 映射: 大块 `IBuffer` + 手动偏移管理

4. **描述符堆管理**: TextureMemoryManager 管理 SRV/RTV/DSV 描述符
   - 设计思想: 集中管理描述符，减少分配
   - Diligent 映射: Diligent 自动管理 SRB，无需手动描述符堆

5. **Render Layer Stack**: 推入/弹出渲染层，支持后处理
   - 设计思想: 层级渲染目标栈，Filter 作为后处理 Pass
   - Diligent 映射: `ITexture` + `ITextureView`（RTV/DSV），通过 RenderTargetPool 复用

---

## 2. Diligent Engine 映射分析

### 2.1 完整映射表

| RmlUi 功能 | DX12 Backend 实现 | Diligent 映射 | 关键 Diligent API |
|------------|-------------------|---------------|-------------------|
| **CompileGeometry** | 创建 VB/IB，子分配到大 Buffer | 创建 `IBuffer`（USAGE_STATIC） | `IRenderDevice::CreateBuffer` |
| **RenderGeometry** | 设置 RootSig → 绑定 CB → 设置 VB/IB View → DrawIndexed | 设置 PSO → 绑定 SRB → 设置 VB/IB → DrawIndexed | `IDeviceContext::SetPipelineState` → `CommitShaderResources` → `SetVertexBuffers` → `DrawIndexed` |
| **ReleaseGeometry** | 释放子分配块 | 释放 `IBuffer` 引用 | `IBuffer::Release` |
| **LoadTexture** | 解析 TGA → GenerateTexture | 同逻辑（Diligent 不含 TGA 解析） | 复用 RmlUi 的 TGA 解析 |
| **GenerateTexture** | 创建 Texture Resource → 上传像素数据 | 创建 `ITexture` + `TextureSubResData` 一次性上传 | `IRenderDevice::CreateTexture`（带 InitialData） |
| **ReleaseTexture** | 释放 Texture + SRV | 释放 `ITexture` 引用 | `ITexture::Release` |
| **EnableScissorRegion** | 切换 PSO 变体（Scissor Enable/Disable） | 设置 `RasterizerStateDesc::ScissorEnable` | PSO 创建时配置 |
| **SetScissorRegion** | `RSSetScissorRects` | `IDeviceContext::SetScissorRects` | `SetScissorRects(1, &rect, 0, 0)` |
| **SetTransform** | 更新 CB 的 transform 矩阵 | 更新动态 CB 的 transform | `Map` → 写入 → `Unmap` |
| **EnableClipMask** | 切换 Stencil PSO 变体 | 切换 Stencil PSO 变体 | 预创建 Stencil PSO 数组 |
| **RenderToClipMask** | 清除 Stencil → 渲染几何体到 Stencil | `ClearDepthStencil` → 渲染到 Stencil | `IDeviceContext::ClearDepthStencil` |
| **PushLayer** | 创建新 RT → 绑定为当前 RT | 从 RenderTargetPool 获取 RT → `SetRenderTargets` | `RenderTargetPool::Acquire` + `SetRenderTargets` |
| **PopLayer** | 恢复之前的 RT | 归还 RT 到 Pool → 恢复之前的 RT | `RenderTargetPool::Release` + `SetRenderTargets` |
| **CompositeLayers** | Blit Source → Apply Filters → Render to Dest | 类似：Blit → Filter Passes → Composite | 多 Pass 渲染 |
| **CompileFilter** | 解析参数 → 创建 CompiledFilter 结构 | 同逻辑（纯数据结构） | 无 GPU 操作 |
| **CompileShader** | 解析参数 → 创建 CompiledShader 结构 | 同逻辑 + 创建 Diligent Shader | `IRenderDevice::CreateShader` |
| **RenderShader** | 绑定 Gradient/Creation PSO → 设置 CB → Draw | 同逻辑 | `SetPipelineState` → `CommitShaderResources` → `DrawIndexed` |

### 2.2 Pipeline 变体映射

DX12 Backend 的 23 种 PSO 变体，映射到 Diligent 预创建的 `IPipelineState` 数组：

```
ProgramId                          → Diligent PSO 配置
─────────────────────────────────────────────────────────────────
Color_Stencil_Always               → VS_Color + PS_Color, Stencil=ALWAYS
Color_Stencil_Equal                → VS_Color + PS_Color, Stencil=EQUAL
Color_Stencil_Set                  → VS_Color + PS_Color, Stencil=REPLACE(1)
Color_Stencil_SetInverse           → VS_Color + PS_Color, Stencil=REPLACE(0)
Color_Stencil_Intersect            → VS_Color + PS_Color, Stencil=INCR_SAT
Color_Stencil_Disabled             → VS_Color + PS_Color, Stencil=OFF
Texture_Stencil_Always             → VS_Main + PS_Texture, Stencil=ALWAYS
Texture_Stencil_Equal              → VS_Main + PS_Texture, Stencil=EQUAL
Texture_Stencil_Disabled           → VS_Main + PS_Texture, Stencil=OFF
Gradient                           → VS_Main + PS_Gradient, Stencil=ALWAYS
Creation                           → VS_Main + PS_Creation, Stencil=ALWAYS
Passthrough                        → VS_PassThrough + PS_Passthrough
Passthrough_NoDepthStencil         → VS_PassThrough + PS_Passthrough, Depth=OFF
Passthrough_Opacity                → VS_PassThrough + PS_Passthrough, Blend=ALPHA
Passthrough_MSAA                   → VS_PassThrough + PS_Passthrough, MSAA
Passthrough_MSAA_Equal             → VS_PassThrough + PS_Passthrough, MSAA+Stencil=EQUAL
Passthrough_NoBlend                → VS_PassThrough + PS_Passthrough, Blend=OFF
Passthrough_NoBlendAndNoMSAA       → VS_PassThrough + PS_Passthrough, Blend=OFF, MSAA=OFF
ColorMatrix                        → VS_PassThrough + PS_ColorMatrix
BlendMask                          → VS_PassThrough + PS_BlendMask (2 SRV)
Blur                               → VS_Blur + PS_Blur
DropShadow                         → VS_PassThrough + PS_DropShadow
```

### 2.3 资源生命周期映射

| 资源类型 | DX12 管理方式 | Diligent 管理方式（v3） |
|----------|-------------|----------------------|
| VB/IB | 子分配到大 Buffer，引用计数 | **一 Geometry 一 Buffer**（USAGE_STATIC） |
| Constant Buffer | 子分配 + 动态 Map | `USAGE_DYNAMIC` Buffer + Map/Unmap |
| Texture | Placed/Committed 策略 | 独立 `ITexture`，Diligent 自动管理 |
| SRV | 描述符堆分配 | `ITexture::GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)` |
| SRB | RootSig 绑定 | **临时创建**，RenderGeometry 时从 PSO 创建并绑定 |
| RTV/DSV | 描述符堆分配 | `ITextureView`（RTV/DSV），通过 RenderTargetPool 复用 |
| PSO | 预创建 23 个 | 预创建 `IPipelineState` 数组 |

---

## 3. 实验架构设计

### 3.1 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        RmlUi Core                               │
│              Rml::Context / Rml::Element / ...                  │
│                     ↓ 调用                                      │
│              Rml::RenderInterface (纯虚接口)                    │
├─────────────────────────────────────────────────────────────────┤
│                RmlDiligentRenderInterface                       │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  实现 Rml::RenderInterface                                │  │
│  │                                                           │  │
│  │  GeometryHandle   ← 编译后的几何体                        │  │
│  │  TextureHandle    ← GPU 纹理                              │  │
│  │  FilterHandle     ← 编译后的滤镜                          │  │
│  │  ShaderHandle     ← 编译后的 Shader                       │  │
│  │  LayerHandle      ← 渲染层                                │  │
│  │                                                           │  │
│  │  PipelineManager  ← 管理 PSO 变体数组                     │  │
│  │  DynamicCB        ← 动态常量缓冲区                        │  │
│  │  RenderTargetPool ← RT 复用池                             │  │
│  │  LayerStack       ← 管理渲染层栈（使用 RT Pool）           │  │
│  └───────────────────────────────────────────────────────────┘  │
│                     ↓ 使用                                      │
│            Diligent Engine 接口（不暴露给 RmlUi）               │
├─────────────────────────────────────────────────────────────────┤
│                    Diligent Engine                               │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  IRenderDevice    ← 资源创建                              │  │
│  │  IDeviceContext   ← 命令录制                              │  │
│  │  ISwapChain       ← 交换链                                │  │
│  │                                                           │  │
│  │  IBuffer / ITexture / ITextureView / IPipelineState       │  │
│  │  IShaderResourceBinding                                  │  │
│  └───────────────────────────────────────────────────────────┘  │
│                     ↓ 抽象                                      │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  D3D11 后端 │ D3D12 后端 │ Vulkan 后端 │ GL 后端 │ Metal │  │
│  └───────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 核心类设计（v3 修订）

```
RmlDiligentRenderInterface : public Rml::RenderInterface
│
├── PipelineManager              // PSO 变体管理
│   ├── m_PSOs[ProgramId]        // 预创建的 PSO 数组（23 个）
│   ├── m_VS_Main / m_VS_PassThrough / m_VS_Blur   // Vertex Shaders
│   └── m_PS_Color / m_PS_Texture / m_PS_Passthrough / m_PS_Blur / ...  // Pixel Shaders
│
├── DynamicCB                    // 动态常量缓冲区（USAGE_DYNAMIC）
│   ├── m_MainCB                 // 主 CB: transform + translate
│   ├── m_BlurCB                 // Blur CB: weights, texelOffset
│   ├── m_GradientCB             // Gradient CB: stops, type
│   ├── m_ColorMatrixCB          // ColorMatrix CB: color matrix
│   └── m_DropShadowCB           // DropShadow CB: color, offset
│
├── RenderTargetPool             // RT 复用池（v4: PooledRT + custom deleter）
│   ├── Acquire(format, w, h)   // 获取一个 RT（优先复用）
│   ├── Release(rt)             // 归还 RT
│   └── m_FreeList[]            // 空闲 RT 列表
│
├── LayerStack                   // 渲染层栈（v4: 使用 PooledRT）
│   ├── Layer[]                  // 每层: ColorRT + DepthStencilRT（从 Pool 获取）
│   ├── m_PostProcessPrimary     // 后处理主 RT（从 Pool 获取）
│   └── m_PostProcessSecondary   // 后处理副 RT（从 Pool 获取）
│
├── 状态管理
│   ├── m_pDevice                // IRenderDevice*
│   ├── m_pContext               // IDeviceContext*
│   ├── m_pSwapChain             // ISwapChain*
│   ├── m_ProjectionMatrix       // 投影矩阵
│   ├── m_TransformMatrix        // 当前变换矩阵
│   ├── m_ScissorEnabled         // 裁剪状态
│   ├── m_ScissorRect            // 当前裁剪矩形
│   ├── m_StencilEnabled         // 模板状态
│   ├── m_StencilDepth           // v4: 模板深度计数（嵌套 Clip 递增）
│   └── m_IsStencilEqual         // 是否使用 EQUAL 测试
│
└── 句柄类型（v3 修订：不暴露 Diligent 对象给 RmlUi）
    ├── GeometryHandle {
    │       RefCntAutoPtr<IBuffer> vertexBuffer;  // 一 Geometry 一 Buffer
    │       RefCntAutoPtr<IBuffer> indexBuffer;
    │       uint32_t vertexCount;
    │       uint32_t indexCount;
    │   }
    ├── TextureHandle {
    │       RefCntAutoPtr<ITexture> texture;
    │       RefCntAutoPtr<ITextureView> SRV;      // 只持有 SRV
    │       // ❌ 不持有 SRB（SRB 由 RenderGeometry 临时创建）
    │   }
    ├── FilterHandle   { FilterType, Sigma, Color, Offset, ColorMatrix }
    ├── ShaderHandle   { CompiledShaderType, GradientParams }
    └── LayerHandle    { unique_ptr<PooledRT> colorRT, unique_ptr<PooledRT> depthRT }
```

### 3.3 v3 关键设计决策

#### 3.3.1 一 Geometry 一 Buffer（删除 BufferManager）

**原因**: RmlUi 的 Geometry 生命周期很长（HTML 加载后缓存），不是 ImGui 那种每帧重建。
一 Geometry 一 Buffer 最简单，实验阶段性能够用。

> **⚠️ 仅适用于实验验证**:
> RmlUi 的 Geometry 并非完全长期不变。动画、文本变化、Layout 变化时会频繁
> `ReleaseGeometry` + `CompileGeometry`。如果进入正式 Runtime，需要：
> - `GeometryBufferAllocator`（子分配大块 Buffer）
> - 或 `Dynamic Geometry Arena`（帧分配器）
> 实验阶段不需要，但未来不可误以为这是最终方案。

```cpp
// v2: CompileGeometry 实现
CompiledGeometryHandle CompileGeometry(Span<const Vertex> vertices, Span<const int> indices) {
    auto* handle = new GeometryHandle();

    // 创建 VB（每个 Geometry 独立）
    BufferDesc vbDesc;
    vbDesc.Name = "RmlDiligent VB";
    vbDesc.Size = vertices.size() * sizeof(Vertex);
    vbDesc.BindFlags = BIND_VERTEX_BUFFER;
    vbDesc.Usage = USAGE_STATIC;  // 长期缓存，不需要动态
    BufferData vbData(vertices.data(), vbDesc.Size);
    m_pDevice->CreateBuffer(vbDesc, &vbData, &handle->vertexBuffer);

    // 创建 IB（每个 Geometry 独立）
    BufferDesc ibDesc;
    ibDesc.Name = "RmlDiligent IB";
    ibDesc.Size = indices.size() * sizeof(int);
    ibDesc.BindFlags = BIND_INDEX_BUFFER;
    ibDesc.Usage = USAGE_STATIC;
    BufferData ibData(indices.data(), ibDesc.Size);
    m_pDevice->CreateBuffer(ibDesc, &ibData, &handle->indexBuffer);

    handle->vertexCount = static_cast<uint32_t>(vertices.size());
    handle->indexCount = static_cast<uint32_t>(indices.size());

    return reinterpret_cast<CompiledGeometryHandle>(handle);
}
```

#### 3.3.2 SRB 临时绑定（删除 TextureHandle 中的 SRB）

**原因**: 同一个 Texture 可能用于 PS_Texture、PS_Blur、PS_DropShadow 等不同 PSO。
一个 Texture 挂一个 SRB，后面 Filter 会炸。

```cpp
// v2: RenderGeometry 中的 SRB 绑定
void RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) {
    auto* geom = reinterpret_cast<GeometryHandle*>(geometry);
    auto* ps = m_PipelineManager.GetPSO(currentProgram);

    // 临时创建 SRB（或使用缓存的 SRB）
    if (texture != 0) {
        auto* tex = reinterpret_cast<TextureHandle*>(texture);
        RefCntAutoPtr<IShaderResourceBinding> srb;
        ps->CreateShaderResourceBinding(&srb, true);
        srb->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex")->Set(tex->SRV);
        m_pContext->CommitShaderResources(srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    // Draw...
}
```

> **优化点**: 可以为常用 PSO+Texture 组合缓存 SRB，但实验阶段不需要。

**未来 SRB 缓存策略**（Phase 6 实现）:
```cpp
// TextureHandle 内维护 ProgramId → SRB 的懒创建 Map
struct TextureHandle {
    RefCntAutoPtr<ITexture> texture;
    RefCntAutoPtr<ITextureView> SRV;

    // SRB 缓存：按需懒创建，PSO 数量有限（~23），Texture 数量多
    // 缓存量 = 实际使用的 PSO×Texture 组合，而非全部组合
    std::unordered_map<ProgramId, RefCntAutoPtr<IShaderResourceBinding>> srbCache;

    IShaderResourceBinding* GetOrCreateSRB(ProgramId program, IPipelineState* pso) {
        auto it = srbCache.find(program);
        if (it != srbCache.end()) return it->second;

        RefCntAutoPtr<IShaderResourceBinding> srb;
        pso->CreateShaderResourceBinding(&srb, true);
        srb->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex")->Set(SRV);
        srbCache[program] = srb;
        return srb;
    }
};
// 比全局 PSO×Texture 缓存更高效，因为很多组合不会被使用
```

#### 3.3.3 RenderTargetPool（替代每次 Create）

**原因**: PushLayer + CompositeLayers 会频繁创建/销毁 RT。
Blur 滤镜每帧可能创建 4+ 个 RT，直接爆。

```cpp
// v4: RenderTargetPool（PooledRenderTarget + custom deleter）
class RenderTargetPool {
public:
    struct Key {
        TEXTURE_FORMAT format;
        uint32_t width;
        uint32_t height;
        uint32_t sampleCount;
        BIND_FLAGS bindFlags;

        bool operator==(const Key& other) const {
            return format == other.format && width == other.width &&
                   height == other.height && sampleCount == other.sampleCount &&
                   bindFlags == other.bindFlags;
        }
    };

    // v4: PooledRenderTarget — 析构时自动归还 Pool
    struct PooledRT {
        RefCntAutoPtr<ITexture> texture;
        RefCntAutoPtr<ITextureView> RTV;
        RefCntAutoPtr<ITextureView> DSV;

        // 析构时归还 Pool（类似 shared_ptr + custom deleter）
        ~PooledRT() {
            if (owner) {
                owner->ReturnToPool(this);
            }
        }

        // 禁止拷贝，允许移动
        PooledRT() = default;
        PooledRT(const PooledRT&) = delete;
        PooledRT& operator=(const PooledRT&) = delete;
        PooledRT(PooledRT&& other) noexcept
            : texture(std::move(other.texture)), RTV(std::move(other.RTV)),
              DSV(std::move(other.DSV)), owner(other.owner), key(other.key) {
            other.owner = nullptr;  // 移动后原对象不归还
        }

    private:
        friend class RenderTargetPool;
        RenderTargetPool* owner = nullptr;
        Key key{};
    };

    // 获取 RT（优先复用空闲列表）
    std::unique_ptr<PooledRT> Acquire(const Key& key, bool needDepth = false) {
        // 先查空闲列表
        for (auto it = m_FreeList.begin(); it != m_FreeList.end(); ++it) {
            if ((*it)->key == key) {
                auto rt = std::move(*it);
                m_FreeList.erase(it);
                rt->owner = this;  // 设置 owner，析构时自动归还
                return rt;
            }
        }
        // 没有匹配的，创建新的
        auto rt = CreateNew(key, needDepth);
        rt->owner = this;
        return rt;
    }

private:
    // 归还到空闲列表（由 PooledRT 析构函数调用）
    void ReturnToPool(PooledRT* rt) {
        // 从 unique_ptr 释放所有权，放入空闲列表
        m_FreeList.push_back(std::make_unique<PooledRT>(std::move(*rt)));
    }

    std::vector<std::unique_ptr<PooledRT>> m_FreeList;
    IRenderDevice* m_pDevice;
};

// 使用方式：
// auto rt = pool.Acquire(key);  // unique_ptr<PooledRT>
// SetRenderTargets(rt->RTV, rt->DSV);
// ... 渲染 ...
// rt.reset();  // 析构时自动归还 Pool
// 或者超出作用域自动归还
```

#### 3.3.4 不使用 RenderPass/Framebuffer（v2 删除）

**原因**: 很多 Diligent Sample 都没用 RenderPass。实验阶段用 `SetRenderTargets()` 足够。
否则 DX12/Vulkan 的 RenderPass 兼容层会先把自己绕死。

```cpp
// v2: 绑定 Render Target（不用 BeginRenderPass）
void SetRenderTarget(ITextureView* RTV, ITextureView* DSV = nullptr) {
    m_pContext->SetRenderTargets(1, &RTV, DSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}
```

#### 3.3.5 不调用 Flush（v2 删除）

**原因**: 正常流程 Draw → Draw → Draw → Present，不需要 Flush。
频繁 Flush 会强制驱动提交命令，直接掉性能。

```cpp
// v2: CompositeLayers 不调用 Flush
void CompositeLayers(...) {
    // Blit Source → PostProcess Primary
    // 对每个 Filter 执行渲染
    // 渲染到 Dest Layer
    // ❌ 删除 m_pContext->Flush()
}
```

#### 3.3.6 Shader 直接照搬 DX12 Backend

**原因**: CSS Gradient（linear/radial/conic/repeating + color stop + spread mode + premultiplied alpha）坑非常多。
自己实现 = 无穷无尽的边界情况。

```text
正确做法:
  1. 从 RmlUi_Renderer_DX12.cpp 中提取 HLSL 字符串
  2. 改用 Diligent::ShaderCreateInfo 编译
  3. CB 布局保持与 DX12 Backend 一致（避免重写 Shader 逻辑）
  4. 只改资源绑定方式（RootSig → SRB）
```

```cpp
// v2: Shader 编译（从 DX12 Backend 移植）
void CompileShaders() {
    // 直接使用 DX12 Backend 的 HLSL 源码
    static const char* vsMainSrc = R"(
        // ... 从 RmlUi_Renderer_DX12.cpp 移植的 HLSL
    )";

    ShaderCreateInfo sci;
    sci.Source = vsMainSrc;
    sci.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    sci.HLSLVersion = {5, 0};
    sci.Desc.ShaderType = SHADER_TYPE_VERTEX;
    sci.EntryPoint = "main";
    m_pDevice->CreateShader(sci, &m_VS_Main);
}
```

### 3.3 句柄封装原则

**核心约束**: 禁止暴露 Diligent 对象给 RmlUi。

所有通过 `Rml::CompiledGeometryHandle`、`Rml::TextureHandle` 等返回给 RmlUi 的值，都是指向内部句柄结构体的指针（cast to `uintptr_t`）。RmlUi 只能看到不透明的整数值。

```cpp
// RmlUi 看到的
using CompiledGeometryHandle = uintptr_t;  // 不透明指针
using TextureHandle = uintptr_t;           // 不透明指针

// 内部实际类型
struct GeometryHandle {
    RefCntAutoPtr<IBuffer> vertexBuffer;
    RefCntAutoPtr<IBuffer> indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t cbOffset;  // 常量缓冲区偏移
};

struct TextureHandle {
    RefCntAutoPtr<ITexture> texture;
    RefCntAutoPtr<ITextureView> SRV;
    RefCntAutoPtr<IShaderResourceBinding> SRB;
};
```

---

## 4. 渲染流程设计

### 4.1 每帧渲染流程

```
BeginFrame()
│
├── 1. 获取当前 Back Buffer RTV
│   └── m_pSwapChain->GetCurrentBackBufferRTV()
│
├── 2. 清除 Render Target
│   └── m_pContext->ClearRenderTarget(pRTV, ClearColor, ...)
│
├── 3. 清除 Depth/Stencil
│   └── m_pContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG|CLEAR_STENCIL_FLAG, ...)
│
├── 4. 设置 Viewport
│   └── m_pContext->SetViewports(1, &viewport, 0, 0)
│
└── 5. 重置状态
    ├── m_TransformMatrix = Identity
    ├── m_ScissorEnabled = false
    ├── m_StencilEnabled = false
    └── m_IsStencilEqual = false

Rml::Context::Update()
│
└── (RmlUi 内部逻辑，不涉及渲染)

Rml::Context::Render()
│
├── 1. RmlUi 遍历元素，调用 RenderInterface 方法
│
│   对每个可见元素:
│   ├── SetTransform(matrix)     → 更新 m_TransformMatrix
│   ├── EnableScissorRegion(true) → m_ScissorEnabled = true
│   ├── SetScissorRegion(rect)   → m_ScissorRect = rect
│   ├── RenderGeometry(handle, translation, texture)
│   │   ├── 选择 PSO 变体（Texture/Color × Stencil 状态）
│   │   ├── 设置 PSO
│   │   ├── 更新 CB（Projection × Transform, Translation）
│   │   ├── 设置 VB/IB
│   │   ├── 绑定 SRB（如果有纹理）
│   │   ├── 设置 Scissor Rect（如果启用）
│   │   └── DrawIndexedInstanced
│   │
│   ├── EnableClipMask(true)     → 切换到 Stencil PSO
│   ├── RenderToClipMask(op, geometry, translation)
│   │   ├── ClearStencil（如果是 Set/SetInverse）
│   │   ├── 设置 Stencil 参考值
│   │   ├── 渲染遮罩几何体（Color Mask = 0，只写 Stencil）
│   │   └── 后续渲染使用 Stencil_Equal PSO
│   │
│   ├── PushLayer()              → 创建新 RT，绑定为当前 RT
│   ├── CompositeLayers(src, dst, blend, filters)
│   │   ├── Blit Source Layer → PostProcess Primary
│   │   ├── 对每个 Filter:
│   │   │   ├── Passthrough → 设置 Opacity，渲染到 Dest
│   │   │   ├── ColorMatrix → 设置 ColorMatrix CB，渲染到 Dest
│   │   │   ├── Blur → 多 Pass 高斯模糊（Primary ↔ Secondary 交替）
│   │   │   ├── DropShadow → 偏移 + Alpha 采样 + Blur + 合成
│   │   │   └── MaskImage → 双纹理混合
│   │   ├── 将结果渲染到 Dest Layer（Blend/Replace 模式）
│   │   └── ❌ 不调用 Flush（v2 修订）
│   │
│   └── PopLayer()               → 归还 RT 到 RenderTargetPool
│
└── (RmlUi 渲染完成)

EndFrame()
│
├── 1. Present
│   └── m_pSwapChain->Present(0 或 1)
│
└── 2. 清理临时资源（如果有）
```

### 4.2 Diligent 调用序列示例（v3 修订）

```cpp
// v2: RenderGeometry 具体调用序列
void RmlDiligentRenderInterface::RenderGeometry(
    CompiledGeometryHandle geometry,
    Vector2f translation,
    TextureHandle texture)
{
    auto* geom = reinterpret_cast<GeometryHandle*>(geometry);

    // 1. 选择 PSO
    ProgramId program = SelectProgram(texture != 0, m_StencilEnabled, m_IsStencilEqual);
    auto* ps = m_PipelineManager.GetPSO(program);
    m_pContext->SetPipelineState(ps);

    // 2. 更新常量缓冲区（Dynamic CB）
    {
        MapHelper<MainCB> cb(m_pContext, m_MainCB, MAP_WRITE, MAP_FLAG_DISCARD);
        cb->transform = m_ProjectionMatrix * m_TransformMatrix;
        cb->translate = {translation.x, translation.y};
    }

    // 3. 绑定资源（v2: SRB 临时创建）
    if (texture != 0) {
        auto* tex = reinterpret_cast<TextureHandle*>(texture);

        // 从 PSO 创建 SRB
        RefCntAutoPtr<IShaderResourceBinding> srb;
        ps->CreateShaderResourceBinding(&srb, true);

        // 绑定纹理 SRV
        auto* var = srb->GetVariableByName(SHADER_TYPE_PIXEL, "g_tex");
        if (var) var->Set(tex->SRV);

        // 提交
        m_pContext->CommitShaderResources(srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    // 4. 设置 VB/IB（v2: 无偏移，一 Geometry 一 Buffer）
    IBuffer* pVB = geom->vertexBuffer;
    IBuffer* pIB = geom->indexBuffer;
    constexpr Uint64 offset = 0;
    m_pContext->SetVertexBuffers(0, 1, &pVB, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pContext->SetIndexBuffer(pIB, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // 5. 设置 Scissor
    if (m_ScissorEnabled) {
        Rect scissorRect = ConvertToDiligentRect(m_ScissorRect);
        m_pContext->SetScissorRects(1, &scissorRect, 0, 0);
    }

    // 6. Draw
    DrawIndexedAttribs drawAttrs(geom->indexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL);
    m_pContext->DrawIndexed(drawAttrs);
}
```

---

## 5. Shader 方案

### 5.1 Shader 清单

所有 Shader 使用 HLSL 编写，通过 Diligent 内置编译器编译为各后端格式。

#### Vertex Shaders

| Shader 名称 | 功能 | 输入 | 输出 |
|-------------|------|------|------|
| `VS_Main` | 主 Vertex Shader | position(float2), color(float4), uv(float2) | SV_Position, color, uv |
| `VS_PassThrough` | 直通（全屏四边形） | position(float2), color(float4), uv(float2) | SV_Position, uv（翻转 Y） |
| `VS_Blur` | Blur 专用（预计算 UV 偏移） | position(float2), color(float4), uv(float2) | SV_Position, uv[BLUR_SIZE], weights |

#### Pixel Shaders

| Shader 名称 | 功能 | 关键逻辑 |
|-------------|------|---------|
| `PS_Color` | 纯颜色 | `return input.color;` |
| `PS_Texture` | 纹理 × 颜色 | `return tex.Sample(samp, uv) * color;` |
| `PS_Passthrough` | 纹理直通 | `return tex.Sample(samp, uv);` |
| `PS_Blur` | 7-Tap 高斯模糊 | 加权采样，Clamp 到 texCoordMin/Max |
| `PS_DropShadow` | 阴影 | Alpha 采样 × 阴影颜色 |
| `PS_ColorMatrix` | 色彩矩阵变换 | `mul(colorMatrix, tex.Sample(...))` |
| `PS_BlendMask` | 双纹理混合 | `source.rgb * mask.a` |
| `PS_Gradient` | 渐变（线性/径向/锥形） | 16 色停 + Smoothstep 插值 |
| `PS_Creation` | 程序化效果 | 数学函数生成 |

### 5.2 Constant Buffer 布局

```hlsl
// 主 CB（VS_Main / PS_Texture / PS_Color 共用）
cbuffer MainCB : register(b0) {
    float4x4 m_transform;    // Projection × Transform
    float2   m_translate;    // 平移偏移
};

// Blur CB（VS_Blur / PS_Blur 共用）
cbuffer BlurCB : register(b0) {
    float4x4 m_transform;
    float2   m_translate;
    float2   m_texelOffset;     // 纹素偏移
    float4   m_weights[2];      // 4 个高斯权重
    float4   m_texCoordMin;     // UV 范围限制
    float4   m_texCoordMax;
};

// Gradient CB（PS_Gradient）
cbuffer GradientCB : register(b0) {
    float4x4 m_transform;
    float2   m_translate;
    int      m_numStops;
    int      m_gradientType;     // 0=linear, 1=radial, 2=conic, 3=repeating
    float2   m_p1, m_p2;         // 渐变控制点
    float4   m_stops[16];        // 色停 (offset, r, g, b)
};

// ColorMatrix CB（PS_ColorMatrix）
cbuffer ColorMatrixCB : register(b0) {
    float4x4 m_transform;
    float2   m_translate;
    float4x4 m_colorMatrix;      // 4×4 色彩变换矩阵
};

// DropShadow CB（PS_DropShadow）
cbuffer DropShadowCB : register(b0) {
    float4x4 m_transform;
    float2   m_translate;
    float4   m_shadowColor;      // 阴影颜色
    float2   m_shadowOffset;     // 阴影偏移
    float2   m_texCoordMin;
    float2   m_texCoordMax;
};
```

### 5.3 Shader 管理方案

```
ShaderManager
├── 编译阶段（初始化时）
│   ├── 从内嵌 HLSL 字符串编译所有 Shader
│   ├── 使用 Diligent::ShaderCreateInfo
│   ├── SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL
│   ├── HLSLVersion = {5, 0}（兼容性最佳）
│   └── 存储 RefCntAutoPtr<IShader>
│
├── PSO 创建阶段（初始化时）
│   ├── 组合 VS + PS + 状态 → 创建 IPipelineState
│   ├── 为每个 PSO 创建 IShaderResourceBinding 模板
│   └── 存储到 PipelineManager
│
└── 运行时
    ├── 按 ProgramId 索引 PSO
    ├── 更新 CB（Map/Unmap）
    └── 绑定 SRB（纹理/CB 变量）
```

### 5.4 Custom Shader 支持

RmlUi 的 `CompileShader` 接口支持渐变和程序化效果：

```cpp
CompiledShaderHandle CompileShader(const String& name, const Dictionary& parameters) {
    if (name == "gradient") {
        // 解析参数：gradient-type, stops, p1, p2, ...
        // 创建 CompiledShader 结构
        // 无需额外 GPU 资源（使用预编译的 PS_Gradient）
    }
    else if (name == "creation") {
        // 解析参数
        // 使用预编译的 PS_Creation
    }
}

void RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry,
                  Vector2f translation, TextureHandle texture) {
    // 选择 PSO: Gradient 或 Creation
    // 设置对应的 CB 参数
    // 渲染几何体
}
```

---

## 6. Clip Mask 实现方案

### 6.1 Stencil 状态组合

需要预创建以下 Stencil PSO 变体：

| 操作 | StencilFunc | StencilPassOp | StencilRef | ColorWriteMask |
|------|-------------|---------------|------------|----------------|
| 渲染（无遮罩） | ALWAYS | KEEP | 0 | RGBA |
| 渲染（遮罩相等） | EQUAL | KEEP | 1 | RGBA |
| 设置遮罩 | ALWAYS | REPLACE | 1 | 0（禁用颜色写入） |
| 设置反向遮罩 | ALWAYS | REPLACE | 0 | 0（禁用颜色写入） |
| 交集遮罩 | ALWAYS | INCR_SAT | 1 | 0（禁用颜色写入） |
| 渲染（遮罩禁用） | ALWAYS | KEEP | 0 | RGBA |

### 6.2 Diligent Stencil 配置

```cpp
// 示例：设置遮罩 PSO 的 DepthStencilStateDesc
DepthStencilStateDesc dssDesc;
dssDesc.DepthEnable = False;  // UI 不需要深度测试
dssDesc.StencilEnable = True;
dssDesc.StencilReadMask = 0xFF;
dssDesc.StencilWriteMask = 0xFF;
dssDesc.FrontFace.StencilFunc = COMPARISON_FUNC_ALWAYS;
dssDesc.FrontFace.StencilPassOp = STENCIL_OP_REPLACE;
dssDesc.FrontFace.StencilFailOp = STENCIL_OP_KEEP;
dssDesc.FrontFace.StencilDepthFailOp = STENCIL_OP_KEEP;
```

### 6.3 Clip Mask 流程（v4: Stencil Depth 管理）

```
RenderToClipMask(ClipMaskOperation::Set, geometry, translation)
│
├── 1. ClearStencil(0)
│   └── m_pContext->ClearDepthStencil(pDSV, CLEAR_STENCIL_FLAG, 0, 0, ...)
│
├── 2. 设置 Stencil 参考值 = ++m_StencilDepth（v4: 嵌套递增）
│   └── m_pContext->SetStencilRef(m_StencilDepth)
│
├── 3. 切换到 Stencil_Set PSO（Color Mask = 0，只写 Stencil）
│   └── m_pContext->SetPipelineState(m_PSOs[ProgramId::Color_Stencil_Set])
│
├── 4. 渲染遮罩几何体（颜色写入被禁用，只更新 Stencil Buffer）
│   └── RenderGeometry(geometry, translation, 0)
│
└── 5. 后续渲染使用 Stencil_Equal PSO
    └── m_IsStencilEqual = true

嵌套 Clip 示例（A → B → C）:
  A: m_StencilDepth = 1, StencilRef = 1
  B: m_StencilDepth = 2, StencilRef = 2
  C: m_StencilDepth = 3, StencilRef = 3
  渲染: StencilFunc = EQUAL, StencilRef = m_StencilDepth

EnableClipMask(false):
  ├── m_StencilDepth = max(0, m_StencilDepth - 1)
  ├── 如果 m_StencilDepth == 0: m_IsStencilEqual = false
  └── 否则: StencilRef = m_StencilDepth（保持上一层遮罩）
```

### 6.4 Rect Clip vs Geometry Clip

| 类型 | 实现方式 |
|------|---------|
| **Rect Clip**（Scissor） | `IDeviceContext::SetScissorRects`，硬件加速，最高效 |
| **Geometry Clip**（Stencil） | Stencil Buffer，支持任意形状，较慢 |

RmlUi 会根据裁剪区域形状自动选择：
- 矩形区域 → `EnableScissorRegion` + `SetScissorRegion`
- 任意形状 → `EnableClipMask` + `RenderToClipMask`

---

## 7. Filters 实现方案

### 7.1 Filter 类型与实现

#### 7.1.1 Passthrough（Opacity）

```
用途: opacity 滤镜
流程:
  1. Bind PostProcess Primary RT 作为输入纹理
  2. 设置 Blend Factor = opacity
  3. 渲染全屏四边形到目标 RT
Diligent PSO: Passthrough_Opacity（Alpha Blend）
```

#### 7.1.2 ColorMatrix

```
用途: brightness, contrast, invert, grayscale, sepia, hue-rotate, saturate
流程:
  1. 计算 4×4 色彩变换矩阵
  2. 上传到 ColorMatrix CB
  3. Bind PostProcess Primary RT 作为输入纹理
  4. 渲染全屏四边形到目标 RT
Diligent PSO: ColorMatrix
```

#### 7.1.3 Blur

```
用途: blur 滤镜
流程（多 Pass 可分离高斯模糊）:
  1. 计算模糊参数（Sigma → Pass Level + Adjusted Sigma）
  2. 计算高斯权重: exp(-i² / (2σ²)) / (√(2π) × σ)
  3. 归一化权重使总和 = 1
  4. 对每个 Pass:
     a. Horizontal Pass: Primary → Secondary（水平模糊）
     b. Vertical Pass: Secondary → Primary（垂直模糊）
     c. 交替 Primary/Secondary 作为源和目标
  5. BLUR_SIZE = 7 taps, BLUR_NUM_WEIGHTS = 4 unique weights

Diligent PSO: Blur（VS_Blur + PS_Blur）
CB: BlurCB（含 texelOffset, weights, texCoordMin/Max）
```

#### 7.1.4 DropShadow

```
用途: drop-shadow 滤镜
流程:
  1. 偏移 UV 采样源纹理 Alpha
  2. 乘以阴影颜色 → 写入 Secondary RT
  3. 如果 sigma >= 0.5:
     a. 对 Secondary RT 执行 Blur（同上）
  4. 将阴影结果合成到目标 RT

Diligent PSO: DropShadow
CB: DropShadowCB（含 shadowColor, shadowOffset, texCoordMin/Max）
```

#### 7.1.5 MaskImage

```
用途: 遮罩图像混合
流程:
  1. Bind 源纹理 (SRV 0)
  2. Bind 遮罩纹理 (SRV 1)
  3. 渲染: result = source.rgb × mask.a
Diligent PSO: BlendMask（双纹理 SRV）
```

### 7.2 Render Layer Stack（v4 修订：PooledRT 自动归还）

```
LayerStack 管理（v4: 使用 PooledRT，析构自动归还 Pool）:

关键设计: PooledRT 是 unique_ptr + custom deleter，超出作用域自动归还 Pool。
         SaveLayerAsTexture 返回的 TextureHandle 持有 PooledRT 的引用，
         当 TextureHandle 被 ReleaseTexture 时，PooledRT 析构，自动归还 Pool。

PushLayer():
  ├── 从 Pool Acquire Color RT (RGBA8) → unique_ptr<PooledRT>
  ├── 从 Pool Acquire DepthStencil RT (D24S8) → unique_ptr<PooledRT>
  ├── 推入栈（Layer 持有这两个 unique_ptr）
  └── SetRenderTargets(colorRT->RTV, depthRT->DSV)

PopLayer():
  ├── 弹出栈
  ├── DepthStencil RT 超出作用域 → 自动归还 Pool
  ├── Color RT 如果被 SaveLayerAsTexture 引用 → 转移所有权到 TextureHandle
  ├── Color RT 如果未被引用 → 超出作用域 → 自动归还 Pool
  └── 恢复之前的 RT

CompositeLayers(source, destination, blendMode, filters):
  ├── 1. 从 Pool Acquire PostProcessPrimary 和 PostProcessSecondary
  │
  ├── 2. Blit Source Layer → PostProcess Primary
  │
  ├── 3. 对每个 Filter 执行渲染
  │
  ├── 4. 渲染到 Dest Layer（Blend/Replace 模式）
  │
  └── 5. PostProcessPrimary 和 PostProcessSecondary 超出作用域 → 自动归还 Pool

SaveLayerAsTexture():
  ├── 从当前 Layer 移动 Color RT 的 unique_ptr 到 TextureHandle
  ├── 返回 TextureHandle（持有 PooledRT，析构时自动归还 Pool）
  └── 不需要手动管理生命周期

SaveLayerAsMaskImage():
  └── 同 SaveLayerAsTexture

TextureHandle 生命周期（v4: 自动管理）:
  ├── 创建: SaveLayerAsTexture 或 GenerateTexture
  ├── 使用: 作为 RenderGeometry 的 texture 参数
  └── 销毁: RmlUi 调用 ReleaseTexture
      ├── 如果是 Pool 来源: PooledRT 析构 → 自动归还 Pool
      └── 如果是 GenerateTexture 来源: 直接释放 ITexture
```

---

## 8. 实验目录结构（v3 修订）

```
Engine/Source/Experiments/RmlDiligent/
├── CMakeLists.txt                          # 模块构建配置
├── RmlDiligentConfig.h                     # 模块配置头文件
│
├── Public/                                 # 公共接口
│   ├── RmlDiligentRenderInterface.h        # 主接口类声明
│   ├── RmlDiligentTypes.h                  # 句柄类型定义
│   └── RmlDiligentFactory.h                # 创建工厂
│
├── Private/                                # 私有实现
│   ├── RmlDiligentRenderInterface.cpp      # 主接口实现
│   ├── RmlDiligentTypes.cpp                # 句柄管理
│   └── RmlDiligentFactory.cpp              # 工厂实现
│
├── Renderer/                               # 渲染器核心
│   ├── RmlDiligentPipelineManager.h        # PSO 变体管理
│   ├── RmlDiligentPipelineManager.cpp
│   ├── RmlDiligentRenderTargetPool.h       # RT 复用池（v4: PooledRT + custom deleter）
│   ├── RmlDiligentRenderTargetPool.cpp
│   ├── RmlDiligentLayerStack.h             # 渲染层栈
│   ├── RmlDiligentLayerStack.cpp
│   ├── RmlDiligentStateManager.h           # 渲染状态管理
│   └── RmlDiligentStateManager.cpp
│   # ❌ 删除 RmlDiligentBufferManager（v2: 一 Geometry 一 Buffer）
│
├── Shaders/                                # HLSL Shader（v2: 从 DX12 Backend 移植）
│   ├── RmlDiligent_Shaders.h               # 内嵌 HLSL 字符串（从 DX12 Backend 提取）
│   └── RmlDiligent_ShaderCommon.hlsli      # 公共 CB 布局定义
│   # 注意: Shader 源码直接内嵌在 .h 中，与 DX12 Backend 保持一致
│
├── Resources/                              # 资源管理
│   ├── RmlDiligentTextureManager.h         # 纹理管理
│   └── RmlDiligentTextureManager.cpp
│   # ❌ 删除 RmlDiligentGeometryManager（v2: GeometryHandle 简单自管理）
│
└── Tests/                                  # 测试
    ├── TestMain.cpp                        # 测试入口
    ├── TestRmlDemo.cpp                     # Phase 1: 跑通 RmlUi 官方 Demo
    ├── TestTransforms.cpp                  # Phase 2 测试
    ├── TestScissor.cpp                     # Phase 3 测试
    ├── TestClipMask.cpp                    # Phase 4 测试
    ├── TestLayerStack.cpp                  # Phase 5a 测试
    ├── TestFilters.cpp                     # Phase 5b 测试
    ├── TestCustomShader.cpp                # Phase 3 测试
    ├── TestFilters.cpp                     # Phase 5b 测试
    └── TestPerformance.cpp                 # Phase 6 测试
```

---

## 9. 分阶段实施计划（v3 修订）

### Phase 0: Shader 编译 + PSO 创建 + Smoke Test（v4 修订）

**预计时间**: 3-4 天

**目标**: 验证全部 DX12 Backend Shader 能通过 Diligent 编译、创建 PSO、并成功渲染三角形。

> **为什么 Phase 0 最重要**:
> 编译通过 ≠ 运行正确。Diligent 最容易翻车的是 CB Layout / Resource Binding / Semantic，
> 而不是编译。所以 Phase 0 必须包含 Smoke Test。

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| **Phase 0.1: Shader 编译验证** | | |
| 0.1 | 从 RmlUi_Renderer_DX12.cpp 提取所有 HLSL 字符串 | `RmlDiligent_Shaders.h` |
| 0.2 | 编译 VS_Main | ✅ 编译成功 |
| 0.3 | 编译 VS_PassThrough | ✅ 编译成功 |
| 0.4 | 编译 VS_Blur | ✅ 编译成功 |
| 0.5 | 编译 PS_Color | ✅ 编译成功 |
| 0.6 | 编译 PS_Texture | ✅ 编译成功 |
| 0.7 | 编译 PS_Passthrough | ✅ 编译成功 |
| 0.8 | 编译 PS_Blur | ✅ 编译成功 |
| 0.9 | 编译 PS_DropShadow | ✅ 编译成功 |
| 0.10 | 编译 PS_ColorMatrix | ✅ 编译成功 |
| 0.11 | 编译 PS_BlendMask | ✅ 编译成功 |
| 0.12 | 编译 PS_Gradient | ✅ 编译成功 |
| 0.13 | 编译 PS_Creation | ✅ 编译成功 |
| 0.14 | 确认 CB 布局与 DX12 Backend 一致 | 布局验证 |
| **Phase 0.5: PSO 创建验证** | | |
| 0.15 | VS_Main + PS_Color → CreateGraphicsPipelineState | ✅ PSO 创建成功 |
| 0.16 | VS_Main + PS_Texture → CreateGraphicsPipelineState | ✅ PSO 创建成功 |
| 0.17 | 确认 InputLayout（position/color/uv）正确 | Semantic 验证 |
| 0.18 | 确认 CB 绑定正确（ShaderResourceVariable） | 绑定验证 |
| **Phase 0.8: Smoke Test** | | |
| 0.19 | 创建简单三角形（3 顶点，无纹理） | 渲染验证 |
| 0.20 | VS_Main + PS_Color → Draw → 截图验证 | ✅ 彩色三角形显示 |
| 0.21 | 创建带纹理四边形 | 渲染验证 |
| 0.22 | VS_Main + PS_Texture → Draw → 截图验证 | ✅ 纹理四边形显示 |
| 0.23 | 验证 CB 数据正确传递 | transform/translate 生效 |

#### 验证方式

- [x] 全部 12 个 Shader 编译成功（2 个有 warning，无错误） ✅
- [x] CB 布局与 DX12 Backend 的 `ConstantBuffer` 定义一致 ✅
- [x] 纹理变量名与 DX12 Backend 一致 ✅
- [x] PSO 创建成功（Color PSO，使用 RenderPass） ✅
- [x] Smoke Test: 彩色三角形正确显示 ✅（Phase 1）
- [x] Smoke Test: 纹理四边形正确显示（Phase 1.5，层合成 + Texture PSO）✅
- [x] CB 数据（transform/translate）正确传递（Phase 1.5）✅

#### 实施中发现的关键问题

| 问题 | 解决方案 |
|------|----------|
| `UseCombinedTextureSamplers` 默认为 true | 设置为 false（RmlUi HLSL 用独立 texture + sampler） |
| `NumRenderTargets` 使用 RenderPass 时必须为 0 | 设置 `NumRenderTargets = 0`, `RTVFormats = UNKNOWN` |
| NVAPI 需要下载 GitHub 依赖 | 使用 `Stubs/nvapi/` 提供空实现 |
| ATL (atlbase.h) 缺失 | 系统已有 ATL（VS 18 Community），无需额外安装 |
| Visual Studio 版本 | 使用 `-G "Visual Studio 18 2026"` |
| vcpkg SDL3 | 使用 `-DCMAKE_PREFIX_PATH="E:/vcpkg/installed/x64-windows"` |

**为什么 Phase 0 包含 Smoke Test**:
> 编译通过 ≠ 运行正确。很多 HLSL 问题（CB 布局错位、Semantic 不匹配、采样器未绑定）
> 只有在运行时才会暴露。Phase 0 的 Smoke Test 能提前发现这些问题。

---

### Phase 1: 跑通 RmlUi 官方 Demo

**预计时间**: 3-4 天

**目标**: 创建窗口 → Diligent Device → Rml::Context → 加载官方 demo.rml → 显示 Hello World

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 1.1 | 创建 RmlDiligent 模块目录结构 | CMakeLists.txt + 目录树 |
| 1.2 | 定义句柄类型 | `RmlDiligentTypes.h`（GeometryHandle, TextureHandle） |
| 1.3 | 创建 PSO: Color_Stencil_Disabled + Texture_Stencil_Disabled | 基础 PSO（复用 Phase 0 编译的 Shader） |
| 1.4 | 实现 `CompileGeometry` | 一 Geometry 一 Buffer（USAGE_STATIC） |
| 1.5 | 实现 `RenderGeometry`（Color + Texture） | PSO 选择 + CB 更新 + SRB 临时绑定 + Draw |
| 1.6 | 实现 `ReleaseGeometry` | 释放 VB/IB |
| 1.7 | 实现 `GenerateTexture` | 创建 ITexture + TextureSubResData 上传 |
| 1.8 | 实现 `LoadTexture` | 解析 TGA → GenerateTexture |
| 1.9 | 实现 `ReleaseTexture` | 释放 ITexture |
| 1.10 | 实现 `EnableScissorRegion` + `SetScissorRegion` | Scissor 基础支持 |
| 1.11 | 实现投影矩阵 + CB 上传 | 正交投影（2D UI） |
| 1.12 | 集成 Rml::SystemInterface + Rml::FileInterface | 平台接口 |
| 1.13 | 加载官方 RmlUi Demo | demo.rml + demo.rcss + 资源文件 |
| 1.14 | 运行主循环 | Update → Render → Present |

#### 验证方式

- [x] 窗口正常显示 ✅
- [x] 彩色三角形渲染成功（Smoke Test） ✅
- [ ] RmlUi Demo 的文字正确渲染（Phase 1.5）
- [ ] RmlUi Demo 的按钮正确显示和交互（Phase 1.5）
- [ ] RmlUi Demo 的图片正确加载和显示（Phase 1.5）
- [ ] Scissor 裁剪正常工作（Phase 1.5）
- [ ] 基本 CSS 样式生效（颜色、背景、边框）（Phase 1.5）

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| RmlUi 初始化流程不熟悉 | 高 | 参照 RmlUi_Backend_SDL_GL3.cpp 的初始化逻辑 |
| Diligent + SDL3 窗口集成 | 中 | 参照 NNRuntimeRenderBootstrap 已有实现 |
| TextureSubResData 上传格式 | 中 | 确认 Diligent 推荐写法 |

---

### Phase 2: Transforms

**预计时间**: 1 天

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 2.1 | 实现 `SetTransform` | 变换矩阵管理 |
| 2.2 | 更新 CB 上传逻辑 | Projection × Transform × Translation |

#### 验证方式

- [x] CSS Transform 属性生效 ✅ 2026-06-03
- [x] 嵌套变换正确叠加 ✅ 2026-06-03

**验收**: `RmlDiligentPhase2Test` + `phase2_transforms.rml`（rotate/scale/translate/nested）；`drawCount=11 textureDraws=5` @ D3D12

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| 矩阵乘法顺序错误 | 中 | 对照 DX12 Backend 的 `m_projection * (*transform)` |

---

### Phase 3: Gradient + Creation（v3 提前）

**预计时间**: 2-3 天

**为什么提前**: Gradient 不依赖 RT Pool / LayerStack / Blur，比 Filter 简单得多。
先验证 Shader 管线，再做复杂的 Filter。

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 3.1 | 实现 `CompileShader` | 解析 Gradient/Creation 参数 ✅ 2026-06-03 |
| 3.2 | 实现 `RenderShader` | 使用 Gradient/Creation PSO 渲染 ✅ 2026-06-03 |
| 3.3 | 实现 `ReleaseShader` | 资源释放 ✅ 2026-06-03 |
| 3.4 | 创建 Gradient PSO | VS_Main + PS_Gradient ✅ 2026-06-03 |
| 3.5 | 创建 Creation PSO | VS_Main + PS_Creation ✅ 2026-06-03 |
| 3.6 | Gradient CB 管理 | 色停数据上传（最大 16 色停） ✅ 2026-06-03 |

#### 验证方式

- [x] 线性渐变正确渲染 ✅ 2026-06-03
- [x] 径向渐变正确渲染 ✅ 2026-06-03
- [x] 锥形渐变正确渲染 ✅ 2026-06-03
- [x] Creation 效果正确渲染 ✅ 2026-06-03

**验收**: `RmlDiligentPhase3Test` + `phase3_gradients.rml`（linear/radial/conic/creation）；`shaderDraws=4 drawCount=12` @ D3D12；默认 ESC 退出，`--frames 5` CI 自动 PASS

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| Gradient CB 布局与 DX12 不一致 | 中 | Phase 0 已验证，此处只做运行时填充 |

---

### Phase 4: Clip Mask（Stencil）

**预计时间**: 2-3 天

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 4.1 | 实现 `EnableClipMask` | Stencil 状态切换 + m_StencilTestValue 管理 ✅ 2026-06-03 |
| 4.2 | 实现 `RenderToClipMask` | Stencil 写入（Set/Inverse/Intersect） ✅ 2026-06-03 |
| 4.3 | 创建 Stencil PSO 变体（6 种） | Color/Texture/Gradient/Creation × Equal/Set/Intersect ✅ 2026-06-03 |
| 4.4 | Stencil Clear 逻辑 | EndRenderPass + ClearStencil（D3D12 兼容） ✅ 2026-06-03 |
| 4.5 | 颜色写入禁用（遮罩渲染时） | Color Mask = 0 ✅ 2026-06-03 |
| 4.6 | 嵌套 Clip 支持 | Intersect + 多 clip 元素（对齐 GL3） ✅ 2026-06-03 |

#### 验证方式

- [x] overflow:hidden / border-radius 裁剪正确渲染 ✅ 2026-06-03
- [x] Set/Inverse/Intersect 操作正确 ✅ 2026-06-03
- [x] 嵌套 overflow 裁剪正确 ✅ 2026-06-03

**验收**: `RmlDiligentPhase4Test` + `phase4_clip_mask.rml`；`clipMaskDraws=4 drawCount=10` @ D3D12；默认 ESC 退出，`--frames 5` CI 自动 PASS

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| Stencil Clear 时机错误 | 中 | 对照 DX12 Backend 的 Clear 逻辑 |
| Stencil Ref 值管理 | 中 | 使用 m_StencilDepth 嵌套递增 |

---

### Phase 5: Filters

**预计时间**: 3-4 天（**5a** LayerStack + RT Pool ✅ 2026-06-03；**5b** Filter 渲染 ✅ 2026-06-03）

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 5.1 | 实现 RenderTargetPool | RT 复用池（PooledRT + custom deleter） ✅ 2026-06-03（Phase 5a） |
| 5.2 | 实现 `CompileFilter` / `ReleaseFilter` | 解析 Filter 参数 ✅ 2026-06-03（Phase 5b） |
| 5.3 | 实现 `PushLayer` / `PopLayer` | Layer Stack（使用 RT Pool + 引用计数） ✅ 2026-06-03（Phase 5a） |
| 5.4 | 实现 `CompositeLayers` | 层合成 + `RenderFilters` 链 ✅ 2026-06-03（Phase 5b 接入 filter 路径） |
| 5.5 | 实现 `SaveLayerAsTexture` / `SaveLayerAsMaskImage` | 层保存（PooledRT + CopyTexture） ✅ 2026-06-03（Phase 5b） |
| 5.6 | 创建 Filter PSO | Passthrough/Blur/DropShadow/ColorMatrix/BlendMask/Opacity ✅ 2026-06-03（Phase 5b） |
| 5.7 | 实现 Blur 多 Pass 逻辑 | 可分离高斯模糊（Primary ↔ Secondary 交替） ✅ 2026-06-03（Phase 5b） |
| 5.8 | 实现 DropShadow 逻辑 | 偏移 + Alpha + Blur ✅ 2026-06-03（Phase 5b） |

**Phase 5a 产出**（`Renderer/RmlDiligentRenderTargetPool`、`Renderer/RmlDiligentLayerStack`）：

- `BeginFrame` / `EndFrame` 对齐 GL3：整帧渲染到 Layer RT，EndFrame Passthrough 到 Swapchain
- 共享 D24S8 Stencil DSV（Clip/绘制路径使用 LayerStack DSV）
- `RmlDiligentPhase5aTest` + `phase5a_layers.rml`（`filter: opacity` / `box-shadow` 触发 PushLayer/Composite 调用路径）

**Phase 5b 产出**（`CompileFilter` / `RenderFilters` / Filter PSO + CB）：

- `CompileFilter`：opacity、blur、drop-shadow、brightness/contrast/invert/grayscale/sepia/hue-rotate/saturate
- Filter PSO：Blur、DropShadow、ColorMatrix、BlendMask、PassthroughOpacity/Replace；PostTertiary + BlendMask RT
- `CompositeLayers`：Blit → `RenderFilters` → Passthrough 到目标层
- `SaveLayerAsTexture`（PooledRT + `CopyTexture` 子区域）、`SaveLayerAsMaskImage`
- `RmlDiligentPhase5bTest` + `phase5_filters.rml`（opacity / blur / drop-shadow / brightness / contrast / 多 filter 叠加）

#### 验证方式

- [x] LayerStack + RT Pool：`pushLayers > 0`、`composites > 0`（`RmlDiligentPhase5aTest --frames 5`）✅ 2026-06-03
- [x] Phase 1.5–4 回归通过（BeginFrame 改动画 layer 后）✅ 2026-06-03
- [x] Filter 链路：`filterRenders > 0`、无 “Could not compile filter”（`RmlDiligentPhase5bTest --frames 5`）✅ 2026-06-03
- [x] opacity / blur / drop-shadow / brightness 面板 layout 正确（`phase5_filters.rml`）✅ 2026-06-03
- [x] 视觉效果人工验收（opacity 混合、blur 边缘虚化、drop-shadow 偏移羽化、brightness 调色）✅ 2026-06-03
- [x] contrast 单滤镜 + `#stack-panel` 多 filter 叠加（`brightness` + `contrast`）layout 与链路（`--case contrast|stack`）✅ 2026-06-03

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| Blur 多 Pass RT 切换复杂 | 高 | 仔细对照 DX12 Backend 的 Primary/Secondary 交替逻辑 |
| SaveLayerAsTexture 生命周期 | 高 | PooledRT custom deleter 确保自动归还 Pool |
| SRB 临时创建性能 | 中 | Phase 6 优化 |

---

### Phase 6: 性能优化 + SRB 缓存

**预计时间**: 2-3 天

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 6.1 | SRB 缓存 | PSO+Texture → SRB 缓存 Map | ✅ |
| 6.2 | 性能基准测试 | TestPerformance A/B（替代 DX12 对比） | ✅ |
| 6.3 | 内存使用分析 | MemoryStats + RT Pool 只读 API | ✅ |
| 6.4 | Draw Call 分析 | drawCount / filterRenders / SRB 命中统计 | ✅ |

#### 验证方式

- [x] SRB 缓存 A/B：开启后 FPS ≥ 关闭 × 1.15（`TestPerformance --compare-ab`）✅ 2026-06-03
- [x] SRB 命中率 > 80%（warmup 后，`phase5_filters.rml`）✅ 2026-06-03
- [x] `MemoryStats` + RT Pool 只读统计（`GetFreeListSize` / `GetActiveAcquireCount`）✅ 2026-06-03
- [x] `TestPerformance.cpp` + `RmlDiligentPhase6Test`；`--no-srb-cache` A/B 开关 ✅ 2026-06-03
- [ ] 与 DX12 Backend 性能差距 < 30%（**后续项**：仓库无独立 DX12 Backend 可执行对比，改为自研 SRB A/B）

#### 产出文件

| 文件 | 说明 |
|------|------|
| `Renderer/RmlDiligentProgramId.h` | ProgramId 枚举（Color/Texture/Blur/…） |
| `Renderer/RmlDiligentSrbCache.h/.cpp` | PSO×SRV×CB 二级 SRB 缓存 + hits/misses |
| `Tests/TestPerformance.cpp` | 帧率/p50/p95、DrawCall、SRB 命中、MemoryStats |

#### 风险点

| 风险 | 影响 | 对策 |
|------|------|------|
| SRB 缓存命中率 | 中 | 监控缓存命中率，调整策略 |
| Diligent 抽象层开销 | 低 | 通过 Profiler 定位 |

---

### Phase 7: 官方 RmlUi Samples（SDL3 + RmlDiligent）

**预计时间**: 2-4 天

#### 依赖

| 项 | 说明 |
|----|------|
| RmlUi submodule | `Engine/Source/ThirdParty/RmlUi`，**tag `6.1`**（与 vcpkg `rmlui` 对齐） |
| 初始化 | `git submodule update --init --recursive` |
| Neverness 阶段测试 RML | `Engine/Source/Experiments/RmlDiligent/TestAssets/`（Phase 1.5–6 仍用 `RML_DILIGENT_TEST_ASSETS_DIR`） |
| vcpkg | `RmlUi::Core`、FreeType 字体引擎、`SDL3`、`RmlUi::Debugger`（可选） |

#### 实现内容

| 任务 | 说明 | 产出 |
|------|------|------|
| 7.1 | `rmlui_backend_diligent` | `Backend::` + `RmlDiligentRenderInterface` + `RmlDiligent_AppCommon` |
| 7.2 | `rmlui_shell_diligent` | 上游 `Samples/shell` + Win32 `FindSamplesRoot` |
| 7.3 | SDL3 平台 | `RmlDiligent_SystemSDLUnity.cpp`（`RMLUI_SDL_VERSION_MAJOR=3` + 上游 `RmlUi_Platform_SDL.cpp`） |
| 7.4 | CMake `basic/` 示例 | `RML_DILIGENT_BUILD_OFFICIAL_SAMPLES`（默认 ON） |
| 7.5 | 冒烟测试 | `RmlDiligentSamplesSmokeTest`：`assets/demo.rml`，`drawCount > 0` |

#### 可执行文件（`Samples/basic/`）

| 目标 | 说明 |
|------|------|
| `RmlDiligent_sample_bitmap_font` | 无 FreeType 也可运行（优先 smoke） |
| `RmlDiligent_sample_load_document` | 加载 `assets/demo.rml` |
| `RmlDiligent_sample_demo` | 完整 Demo + F8 Debugger |
| `RmlDiligent_sample_effects` | 滤镜回归（Phase 5b 目视） |
| `RmlDiligent_sample_animation` … `tree_view` | 其余 basic 示例子目录 |
| `RmlDiligentSamplesSmokeTest` | CI：`--frames N`，控制台 `main` |

**未纳入**: `invaders/`、`tutorial/`、`lua_invaders/`；`harfbuzz`（需 vcpkg harfbuzz）；`ime`（Win32 字体辅助源，SDL3 路径暂跳过）。

#### 构建与运行

```powershell
cmake -S Engine/Source/Experiments/RmlDiligent -B Build/RmlDiligent `
  -DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build Build/RmlDiligent --config Debug --target RmlDiligentSamplesSmokeTest
cd Build/RmlDiligent/Debug
.\RmlDiligentSamplesSmokeTest.exe --frames 3
```

POST_BUILD 将 submodule 的 `Samples/` 树复制到**可执行文件目录**（使 `assets/rml.rcss` 与 exe 同级，满足 `Shell::FindSamplesRoot`）。同时复制 SDL3 / RmlUi DLL。

关闭全部官方示例：`cmake -DRML_DILIGENT_BUILD_OFFICIAL_SAMPLES=OFF`。

#### 验证方式

- [x] `rmlui_backend_diligent` / `rmlui_shell_diligent` 编译通过
- [x] `RmlDiligentSamplesSmokeTest --frames 3`：`drawCount > 0`、无崩溃
- [ ] 目视：`RmlDiligent_sample_demo`、`RmlDiligent_sample_effects`
- [x] Phase 1.5–6 测试仍指向 `TestAssets/`

#### 已知限制

- 无官方 DX12 Backend 并行性能对比
- submodule 须与 vcpkg **同为 6.1**；升级 vcpkg 时同步 bump submodule tag
- `ime` / plugin 类 sample（`lottie`/`svg`）未在本阶段注册

---

## 10. 与 NNRuntimeDiligent 的关系

RmlDiligent 是一个**独立实验模块**，不依赖 NNRuntimeDiligent。

```
NNRuntimeDiligent          RmlDiligent
├── 通用渲染架构            ├── RmlUi 专用渲染器
├── 接口层 + 后端层          ├── 直接使用 Diligent API
├── 面向引擎 Renderer2D     ├── 面向 RmlUi RenderInterface
└── C# Handle 互操作        └── 不涉及 C# 层

共同点:
├── 都使用 Diligent Engine 作为 GPU 后端
├── 都需要处理 Shader 编译、PSO 创建、资源管理
└── 都可以从 Diligent 的跨平台能力获益

可能的未来整合:
├── RmlDiligent 可以使用 NNRuntimeRender 接口
├── 或者保持独立（RmlUi 渲染路径独立于主引擎）
└── 取决于实验验证结果
```

---

## 11. CMakeLists.txt 配置（v3 修订）

```cmake
# RmlDiligent 模块
add_library(RmlDiligent STATIC
    Public/RmlDiligentRenderInterface.h
    Public/RmlDiligentTypes.h
    Public/RmlDiligentFactory.h
    Private/RmlDiligentRenderInterface.cpp
    Private/RmlDiligentTypes.cpp
    Private/RmlDiligentFactory.cpp
    Renderer/RmlDiligentPipelineManager.h
    Renderer/RmlDiligentPipelineManager.cpp
    Renderer/RmlDiligentRenderTargetPool.h      # v4: PooledRT + custom deleter
    Renderer/RmlDiligentRenderTargetPool.cpp
    Renderer/RmlDiligentLayerStack.h
    Renderer/RmlDiligentLayerStack.cpp
    Renderer/RmlDiligentStateManager.h
    Renderer/RmlDiligentStateManager.cpp
    Shaders/RmlDiligent_Shaders.h               # v2: 内嵌 HLSL 字符串
    Shaders/RmlDiligent_ShaderCommon.hlsli
    Resources/RmlDiligentTextureManager.h
    Resources/RmlDiligentTextureManager.cpp
    # ❌ 删除 BufferManager（v2: 一 Geometry 一 Buffer）
    # ❌ 删除 GeometryManager（v2: GeometryHandle 简单自管理）
)

target_include_directories(RmlDiligent PUBLIC Public)
target_include_directories(RmlDiligent PRIVATE Private Renderer Resources Shaders)

# 依赖（v3: 只依赖 Core，不依赖 DiligentFX）
target_link_libraries(RmlDiligent
    PUBLIC
        DiligentGraphicsEngine  # 只需要 Core（IBuffer/ITexture/IPipelineState/IShader）
    PRIVATE
        RmlUi::Core
        SDL3::SDL3              # 平台窗口（可选）
)
# ❌ 不依赖 DiligentFX（实验阶段不需要，避免绑定）

# 测试程序
add_executable(RmlDiligentTest Tests/TestMain.cpp Tests/TestRmlDemo.cpp)
target_link_libraries(RmlDiligentTest PRIVATE RmlDiligent)
```

---

## 12. 验收标准（v4 修订）

- [x] **Phase 0**: Shader 编译 + PSO 创建 + Smoke Test 全部通过 ✅ 2026-06-02
  - 12/12 DX12 Backend HLSL Shader 编译成功
  - Color PSO 创建成功（RenderPass + SV_VertexID）
  - Texture PSO 跳过（VS/PS 语义不匹配，Phase 1 验证）
  - 后端: D3D12 (NVIDIA RTX 2070)
- [x] **Phase 1**: SDL3 窗口 + Smoke Test 三角形 ✅ 2026-06-02
  - SDL3 窗口创建成功
  - Diligent D3D12 设备初始化成功
  - 彩色三角形渲染成功
- [x] **Phase 1.5**: RmlUi 基础集成验证 ✅ 2026-06-02
  - [x] SDL3 窗口 + Diligent D3D12 设备 ✅
  - [x] RmlUi 初始化 + SetRenderInterface ✅
  - [x] FileInterface 实现 ✅
  - [x] Color PSO 创建成功（RenderPass + SV_VertexID） ✅
  - [x] Texture PSO 创建成功（VS 输出 float4 Color 匹配 PS 期望）✅
  - [x] Rml::CreateContext 成功 ✅
  - [x] RML 文档加载成功 ✅
  - [x] 字体加载成功（FileInterface 路径）✅
  - [x] LogMessage 回调修复（try-catch 保护空引用）✅
  - [x] 渲染循环运行正常 ✅
  - [x] RmlUi 内容实际渲染（CB/SRB、矩阵 row_major、层合成、inline RML 冒烟）✅ 2026-06-03
  - [x] 预乘 Alpha 混合（ONE / ONE_MINUS_SRC_ALPHA，对齐 GL3）✅ 2026-06-03
  - [x] 自动化验收：`textureDraws > 0`（字体 glyph 路径）✅ 2026-06-03
- [x] **Phase 2**: CSS Transform 属性正确生效 ✅ 2026-06-03
  - [x] `SetTransform`：`m_Projection * (*transform)`（对齐 GL3）✅
  - [x] `BeginFrame` 调用 `SetTransform(nullptr)` 重置基线 ✅
  - [x] `TestTransforms.cpp` + `phase2_transforms.rml` 验收通过 ✅
  - [x] 默认 ESC 退出，`--frames N` 用于 CI 自动退出 ✅
- [x] **Phase 3**: Stencil 遮罩正确（Set/Inverse/Intersect，嵌套 Clip） ✅ 2026-06-03（对应 §9 Phase 4）
  - [x] D24S8 depth/stencil + RenderPass 双附件 ✅
  - [x] `EnableClipMask` / `RenderToClipMask`（对齐 GL3） ✅
  - [x] Stencil Equal/Set/Intersect PSO 变体 ✅
  - [x] `TestClipMask.cpp` + `phase4_clip_mask.rml` 验收通过 ✅
- [x] **Phase 4**: 渐变和程序化效果正确渲染 ✅ 2026-06-03（对应 §9 Phase 3）
  - [x] `CompileShader` / `RenderShader` / `ReleaseShader` ✅
  - [x] Gradient PSO + Creation PSO + CB 上传 ✅
  - [x] `TestCustomShader.cpp` + `phase3_gradients.rml` 验收通过 ✅
- [x] **Phase 5a**: LayerStack + RenderTargetPool + 无 filter 的 CompositeLayers（§9 Phase 5 的 5.1 / 5.3 / 5.4）✅ 2026-06-03
  - [x] `RenderTargetPool`（PooledRT + PoolDeleter）+ `RenderLayerStack`（多层 color RT、共享 DSV、PostPrimary/Secondary）✅
  - [x] `BeginFrame` / `EndFrame` 对齐 GL3（layer 渲染 → EndFrame Passthrough 到 Swapchain）✅
  - [x] `m_PSO_Passthrough` + `DrawFullscreenPassthrough` / `BlitLayerToPostprocessPrimary` ✅
  - [x] `TestLayerStack.cpp` + `phase5a_layers.rml`；`pushLayers=10 composites=5` @ `--frames 5` ✅
- [x] **Phase 5b**: Filters 全链路（§9 Phase 5 的 5.2 / 5.5–5.8）✅ 2026-06-03
  - [x] `CompileFilter` / `ReleaseFilter` / `SaveLayerAsMaskImage` / `SaveLayerAsTexture` ✅
  - [x] Filter PSO + CB（Blur/DropShadow/ColorMatrix/BlendMask/Opacity）✅
  - [x] `RenderFilters` + `RenderBlur`；`CompositeLayers` 接入 filter 链 ✅
  - [x] LayerStack 扩展 PostTertiary + BlendMask ✅
  - [x] `TestFilters.cpp` + `phase5_filters.rml`；`filterRenders=15` @ `--frames 5` ✅
  - [x] Phase 1.5–5a 回归通过 ✅
- [x] **Phase 6**: SRB 缓存 + 性能验收 + 中文注释 ✅ 2026-06-03
  - [x] `TextureHandle.srbCache` + `SrbCache`（PSO×SRV）消除 10 处临时 CreateSRB ✅
  - [x] `MemoryStats` / `ResetPerfStats` / `SetSrbCacheEnabled` ✅
  - [x] `TestPerformance.cpp`：`--frames` / `--warmup` / `--no-srb-cache` / `--compare-ab` ✅
  - [x] 核心模块中文注释（RenderInterface 分节、LayerStack、RT Pool、Shaders 文件头）✅
  - [ ] 与 DX12 Backend 差距 < 30%（后续独立对比项；当前以 SRB A/B 代替）
- [x] **Phase 7**: 官方 Samples（SDL3 + RmlDiligent Backend）✅ 2026-06-04
  - [x] RmlUi submodule **6.1** + `TestAssets/` 迁移 ✅
  - [x] `rmlui_backend_diligent` / `rmlui_shell_diligent` + `basic/` 全部目标（除 `ime`/`harfbuzz`）✅
  - [x] `RmlDiligentSamplesSmokeTest` 自动化冒烟 ✅
  - [ ] 目视 `demo` / `effects`（开发机手动）

---

## 13. 风险与对策总结（v4 修订）

| 风险 | 概率 | 影响 | 对策 |
|------|------|------|------|
| HLSL 编译兼容性 | 中 | 高 | Phase 0 提前验证，逐个修复 |
| CB 布局/Semantic 不匹配 | 中 | 高 | Phase 0 Smoke Test 验证 |
| RmlUi 初始化流程复杂 | 中 | 高 | 参照官方 Backend 的初始化逻辑 |
| Diligent 某些后端不支持特定功能 | 中 | 高 | 优先 Vulkan/DX12 后端，GL 作为回退 |
| Blur 多 Pass RT 切换复杂 | 高 | 中 | 仔细对照 DX12 Backend 的 Primary/Secondary 交替逻辑 |
| SRB 临时创建性能 | 中 | 低 | Phase 6 缓存优化 |
| 性能不如原生 Backend | 中 | 中 | 后期优化，先保证功能正确 |

---

## 14. 后续展望

- **集成到主引擎**: 验证通过后，可作为 RmlUi 的正式 Backend
- **与 NNRuntimeDiligent 整合**: 提取公共 Diligent 工具代码
- **Shader 热重载**: 支持运行时重新编译 Shader
- **MSAA 支持**: 添加多重采样抗锯齿
- **性能 Profiling**: 集成 Diligent 的 GPU 性能计数器
- **多窗口支持**: 支持多个 RmlUi Context 渲染到不同窗口

---

> **下一步**: 审查通过后，开始 Phase 1 实施。
