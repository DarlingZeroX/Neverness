# NNRuntimeDiligent — 渲染架构计划 v4（定稿）

> **状态**: ✅ 可实施  
> **目录**: `Engine/Source/Experiments/`  
> **目标**: 后端无关的渲染架构，Diligent Engine 作为首个后端  
> **创建日期**: 2026-06-01  
> **定稿日期**: 2026-06-02（v4 架构定稿）

---

## 0. 版本演进

| 版本 | 评分 | 关键变更 |
|------|------|---------|
| v1 | 60/100 | 初始设计，Diligent 类型直接暴露 |
| v2 | 85/100 | 拆分接口层/实现层，新增 CommandList |
| v3 | 90/100 | Material 拆出，Handle 系统，Bootstrap 独立 |
| v4 | 95/100 | Handle 收缩边界，RenderGraph 延后，ResourceBinding 内化 |

---

## 1. 全局架构（最终）

```
┌───────────────────────────────────────────────────────────────────┐
│                     C# Managed Layer                             │
│          Renderer2D / Scene / Scripts / Editor / SRP             │
│      (只认识 Material/Mesh/Texture/Camera/Light)                 │
│      (通过 Handle 持有资源，不保存指针)                           │
├───────────────────────────────────────────────────────────────────┤
│                  NNNativeEngineAPI                                │
│          (C++ ↔ C# 互操作边界)                                   │
│      (NNRef ↔ Handle 转换在此发生，唯一的地方)                   │
├───────────────────────────────────────────────────────────────────┤
│             NNRuntimeRenderAssets (资产层)                        │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  Material          ShaderAsset          ShaderVariant      │  │
│  │  PipelineCache     MaterialInstance     AssetRegistry      │  │
│  └─────────────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────────────┤
│              NNRuntimeRender (渲染接口层)                         │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  INNRenderDevice                                             │  │
│  │  INNCommandList          INNRenderQueue                     │  │
│  │  INNBuffer               INNTexture         INNSampler      │  │
│  │  INNPipelineState        INNRenderTarget    INNSwapChain    │  │
│  ├─────────────────────────────────────────────────────────────┤  │
│  │  RenderGraph             RenderPass                         │  │
│  │  NNRenderHandle (类型定义，不含 Registry)                    │  │
│  └─────────────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────────────┤
│              NNRuntimeCore (核心层)                               │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  NNObjectHandleRegistry   ← 通用 Handle 管理                │  │
│  │  NNRef<T>                 ← 智能指针                        │  │
│  │  NNLog                    ← 日志                            │  │
│  │  NNTypes                  ← 基础类型                        │  │
│  └─────────────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────────────┤
│         NNRuntimeRenderBootstrap (后端引导)                       │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  NNRenderBootstrap::CreateDevice(...)                       │  │
│  │  内部条件编译选择后端                                        │  │
│  └─────────────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────────────┤
│              NNRuntimeDiligent (后端实现)                         │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  NNDiligentDevice            NNDiligentCommandList          │  │
│  │  NNDiligentBuffer            NNDiligentTexture              │  │
│  │  NNDiligentPipeline          NNDiligentSwapChain            │  │
│  │  NNDiligentResourceBinding   (内部使用，不暴露)              │  │
│  └─────────────────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────────────────┤
│                    DiligentCore (第三方库)                        │
│           GraphicsEngine / Vulkan / D3D12 / Metal / GL           │
└───────────────────────────────────────────────────────────────────┘
```

---

## 2. 核心设计决策

### 2.1 Handle 边界收缩

**原则**: C++ 内部保持强类型对象，只有跨语言边界才使用 Handle。

```
C++ 内部（NNRuntimeRender / NNRuntimeDiligent / NNRuntimeRenderAssets）
    ↓ 使用
NNRef<INNTexture>  NNRef<INNBuffer>  NNRef<INNPipelineState>
    ↓ 强类型，编译期检查

════════════════ NNNativeEngineAPI 边界 ════════════════

C# 外部
    ↓ 使用
NNRenderHandle (uint64_t)
    ↓ 运行时检查
```

**NNRuntimeRender 模块**：
- ✅ 定义 `NNRenderHandle` 类型（纯类型定义）
- ✅ 定义 `NNRenderHandleType` 枚举
- ❌ 不包含 `NNObjectHandleRegistry`

**NNRuntimeCore 模块**：
- ✅ 包含 `NNObjectHandleRegistry`（通用 Handle 管理）
- ✅ 所有模块共享

**NNNativeEngineAPI**：
- ✅ 唯一做 Handle ↔ NNRef 转换的地方
- ✅ C# 调用时：Handle → Registry 查找 → NNRef → 调用接口
- ✅ C++ 返回时：NNRef → Registry 注册 → 返回 Handle

### 2.2 RenderDevice 返回对象，不返回 Handle

```cpp
// ✅ 正确：Native API 返回强类型对象
class INNRenderDevice {
public:
    virtual NNRef<INNTexture> CreateTexture(const NNTextureDesc& desc) = 0;
    virtual NNRef<INNBuffer> CreateBuffer(const NNBufferDesc& desc) = 0;
    virtual NNRef<INNShader> CreateShader(const NNShaderDesc& desc) = 0;
    virtual NNRef<INNPipelineState> CreatePipelineState(const NNPipelineStateDesc& desc) = 0;
};

// ❌ 错误：不要这样
class INNRenderDevice {
public:
    virtual NNRenderHandle CreateTexture(...) = 0;  // Handle 污染 Native 层
};

// ✅ 正确：Handle 转换在 NNNativeEngineAPI 边界
// NNNativeEngineAPI.cpp
NNRenderHandle NNE_CreateTexture(NNRenderDeviceHandle dev, int w, int h, int fmt) {
    auto* device = g_HandleRegistry.GetDevice(dev);
    auto tex = device->CreateTexture({w, h, fmt});
    return g_HandleRegistry.Register(NNHandleType::Texture, std::move(tex));
}
```

### 2.3 ResourceBinding 内化

**INNCommandList 不暴露 SetResourceBinding**。Material 负责绑定。

```cpp
// ✅ 正确：CommandList 只管渲染命令
class INNCommandList {
public:
    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void SetPipelineState(NNRef<INNPipelineState> pso) = 0;
    virtual void SetVertexBuffer(NNRef<INNBuffer> buffer, uint32_t slot = 0) = 0;
    virtual void SetIndexBuffer(NNRef<INNBuffer> buffer) = 0;

    virtual void Draw(const NNDrawAttribs& attribs) = 0;
    virtual void DrawIndexed(const NNDrawIndexedAttribs& attribs) = 0;
    virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

    virtual void BeginRenderPass(NNRef<INNRenderTarget> rt) = 0;
    virtual void EndRenderPass() = 0;

    virtual void SetViewports(const NNViewport* vp, uint32_t count) = 0;
    virtual void SetScissorRects(const NNRect* rects, uint32_t count) = 0;

    // ❌ 不要暴露这些
    // virtual void SetResourceBinding(...) = 0;
    // virtual void TransitionTexture(...) = 0;
    // virtual void TransitionBuffer(...) = 0;
};

// ✅ 正确：Material 内部处理绑定
// 渲染流程:
//   Material.Apply(cmd)
//     → 内部设置 PSO
//     → 内部设置 SRB（纹理/CB/Sampler 绑定）
//     → cmd.Draw(...)
```

**为什么**：
- Diligent SRB ≠ Vulkan DescriptorSet ≠ DX12 DescriptorHeap ≠ Metal ArgumentBuffer
- 如果暴露到接口层，未来换后端要改所有绑定代码
- Material 是唯一正确的抽象层

**资源屏障**也内化：
- RenderGraph 编译时自动推导屏障
- 后端实现内部处理屏障转换
- C# 和 CommandList 接口不感知屏障

### 2.4 RenderGraph 延后

```
Phase 1-4: 基础设施先跑起来（Device → Resource → Shader/Pipeline → Runtime 集成）
    ↓
    合并主干，确保 Renderer2D / ImGui / Triangle 都能工作
    ↓
Phase 5: RenderGraph 实验版（增强项，不是基础设施）
    ↓
Phase 6: Material + PipelineCache + ShaderAsset
    ↓
Phase 7: SRP（Scriptable Render Pipeline，C# 层自定义渲染流程）
```

**原因**：RenderGraph 是增强项，不是基础设施。Renderer2D 可以直接通过 CommandList 渲染，后面再替换为 RenderGraph。

---

## 3. 模块详细设计

### 3.1 NNRuntimeCore（核心层）

```
Engine/Source/Experiments/NNRuntimeCore/
├── CMakeLists.txt
├── Include/
│   ├── NNCoreConfig.h
│   ├── NNObject.h                      ← 所有 Runtime 对象统一基类
│   ├── NNRef.h                         ← 智能指针
│   ├── NNLog.h                         ← 日志
│   ├── NNTypes.h                       ← 基础类型（float2, float3, float4, matrix4x4）
│   │
│   └── Handle/
│       ├── NNObjectHandleRegistry.h    ← 通用 Handle 注册表
│       └── NNHandleTypes.h             ← Handle 类型定义
│
├── Source/
│   ├── Handle/
│   │   └── NNObjectHandleRegistry.cpp
│   └── NNLog.cpp
│
└── Tests/
    └── TestHandleRegistry.cpp
```

```cpp
// NNObject.h — 所有 Runtime 对象统一基类
namespace NN::Runtime::Core {

class INNObject {
public:
    virtual ~INNObject() = default;
    virtual uint32_t AddRef() = 0;
    virtual uint32_t Release() = 0;
    virtual uint32_t GetRefCount() const = 0;
};

} // namespace NN::Runtime::Core
```

所有接口继承 INNObject：
- INNTexture : public INNObject
- INNBuffer : public INNObject
- INNShader : public INNObject
- INNPipelineState : public INNObject
- NNMaterial : public INNObject

```cpp
// NNRef.h — 智能指针
template<typename T>
class NNRef {
    T* m_Ptr = nullptr;
public:
    NNRef() = default;
    NNRef(T* ptr) : m_Ptr(ptr) { if (m_Ptr) m_Ptr->AddRef(); }
    NNRef(const NNRef& other) : m_Ptr(other.m_Ptr) { if (m_Ptr) m_Ptr->AddRef(); }
    NNRef(NNRef&& other) : m_Ptr(other.m_Ptr) { other.m_Ptr = nullptr; }
    ~NNRef() { if (m_Ptr) m_Ptr->Release(); }
    T* operator->() const { return m_Ptr; }
    T* Get() const { return m_Ptr; }
    explicit operator bool() const { return m_Ptr != nullptr; }
};
```

```cpp
// NNHandleTypes.h — 纯类型定义
namespace NN::Runtime::Core {

using NNHandle = uint64_t;
constexpr NNHandle NN_INVALID_HANDLE = 0;

// HandleType: 只包含 GPU 资源和资产，不包含运行时对象
enum class NNHandleType : uint16_t {
    Unknown      = 0,
    Texture      = 2,   // GPU 纹理
    Buffer       = 3,   // GPU Buffer（VB/IB/CB/SB）
    Shader       = 4,   // 着色器
    Pipeline     = 5,   // PSO
    Material     = 6,   // 材质
    Mesh         = 7,   // 网格
    RenderTarget = 8,   // Render Target
    // ❌ Device / SwapChain / CommandList 不进 Registry
    //    它们是运行时对象，由 EngineContext 直接持有
};

// Handle 编码: [类型 16bit][序号 32bit][版本 16bit]
inline NNHandle MakeHandle(NNHandleType type, uint32_t index, uint16_t version) {
    return (uint64_t)type << 48 | (uint64_t)index << 16 | version;
}

inline NNHandleType GetHandleType(NNHandle h)    { return (NNHandleType)((h >> 48) & 0xFFFF); }
inline uint32_t    GetHandleIndex(NNHandle h)    { return (h >> 16) & 0xFFFFFFFF; }
inline uint16_t    GetHandleVersion(NNHandle h)   { return h & 0xFFFF; }

} // namespace NN::Runtime::Core
```

```cpp
// NNObjectHandleRegistry.h
namespace NN::Runtime::Core {

class NNObjectHandleRegistry {
public:
    // 注册任意资源，返回 Handle
    NNHandle Register(NNHandleType type, NNRef<INNObject> resource);

    // 获取资源
    INNObject* Get(NNHandle handle);

    // 类型安全获取
    template<typename T>
    T* GetAs(NNHandle handle);

    // 释放
    void Release(NNHandle handle);

    // 热重载：底层资源重建，Handle 不变
    void Replace(NNHandle handle, NNRef<INNObject> newResource);

    // 统计
    size_t GetCount(NNHandleType type) const;

private:
    struct Entry {
        NNHandleType type;
        uint16_t version;
        NNRef<INNObject> resource;
    };
    std::unordered_map<uint32_t, Entry> m_Entries;
    std::unordered_map<NNHandleType, uint32_t> m_NextIndex;
    std::mutex m_Mutex;
};

} // namespace NN::Runtime::Core
```

### 3.2 NNRuntimeRender（渲染接口层）

```
Engine/Source/Experiments/NNRuntimeRender/
├── CMakeLists.txt
├── Include/
│   ├── NNRenderConfig.h
│   ├── NNRenderCore.h
│   │
│   ├── Device/
│   │   ├── INNRenderDevice.h           ← 返回 NNRef<T>，不返回 Handle
│   │   ├── INNSwapChain.h
│   │   └── NNDeviceInfo.h
│   │
│   ├── Resources/
│   │   ├── INNBuffer.h
│   │   ├── INNTexture.h
│   │   └── INNSampler.h
│   │
│   ├── Pipeline/
│   │   ├── INNPipelineState.h
│   │   ├── INNShader.h
│   │   └── INNInputLayout.h
│   │
│   ├── Command/
│   │   ├── INNCommandList.h            ← 不暴露 ResourceBinding / 屏障
│   │   └── INNRenderQueue.h
│   │
│   ├── RenderGraph/
│   │   ├── NNRenderGraph.h
│   │   ├── NNRenderPass.h
│   │   ├── NNResourceNode.h
│   │   └── NNRenderGraphBuilder.h
│   │
│   ├── RenderTarget/
│   │   └── INNRenderTarget.h
│   │
│   └── Handle/
│       └── NNRenderHandle.h            ← 纯类型定义（来自 NNRuntimeCore）
│
├── Source/
│   ├── RenderGraph/
│   │   ├── NNRenderGraph.cpp
│   │   ├── NNRenderPass.cpp
│   │   ├── NNResourceNode.cpp
│   │   └── NNRenderGraphBuilder.cpp
│   └── (接口无 .cpp，纯虚类)
│
└── Tests/
    ├── TestRenderGraph.cpp
    └── TestCommandList.cpp
```

```cpp
// INNRenderDevice.h — 返回强类型对象
namespace NN::Runtime::Render {

class INNRenderDevice {
public:
    virtual ~INNRenderDevice() = default;

    // ✅ 返回 NNRef，不是 Handle
    virtual NNRef<INNBuffer> CreateBuffer(const NNBufferDesc& desc, const void* initialData = nullptr) = 0;
    virtual NNRef<INNTexture> CreateTexture(const NNTextureDesc& desc, const void* initialData = nullptr) = 0;
    virtual NNRef<INNShader> CreateShader(const NNShaderDesc& desc) = 0;
    virtual NNRef<INNPipelineState> CreatePipelineState(const NNPipelineStateDesc& desc) = 0;
    virtual NNRef<INNSampler> CreateSampler(const NNSamplerDesc& desc) = 0;
    virtual NNRef<INNRenderTarget> CreateRenderTarget(const NNRenderTargetDesc& desc) = 0;

    virtual const NNDeviceInfo& GetDeviceInfo() const = 0;
    virtual bool IsFeatureSupported(NNFeature feature) const = 0;

    // 命令列表
    virtual INNCommandList* GetImmediateCommandList() = 0;
    virtual NNRef<INNCommandList> CreateDeferredCommandList() = 0;
};

} // namespace NN::Runtime::Render
```

```cpp
// INNCommandList.h — 不暴露 ResourceBinding 和屏障
namespace NN::Runtime::Render {

class INNCommandList {
public:
    virtual ~INNCommandList() = default;

    // 生命周期
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void Reset() = 0;

    // 管线状态
    virtual void SetPipelineState(NNRef<INNPipelineState> pso) = 0;

    // 资源绑定
    virtual void SetVertexBuffer(NNRef<INNBuffer> buffer, uint32_t slot = 0) = 0;
    virtual void SetIndexBuffer(NNRef<INNBuffer> buffer) = 0;

    // 渲染命令
    virtual void Draw(const NNDrawAttribs& attribs) = 0;
    virtual void DrawIndexed(const NNDrawIndexedAttribs& attribs) = 0;
    virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

    // Render Pass
    virtual void BeginRenderPass(NNRef<INNRenderTarget> rt) = 0;
    virtual void EndRenderPass() = 0;

    // 视口 / 裁剪
    virtual void SetViewports(const NNViewport* viewports, uint32_t count) = 0;
    virtual void SetScissorRects(const NNRect* rects, uint32_t count) = 0;

    // ❌ 不暴露
    // virtual void SetResourceBinding(...) = 0;
    // virtual void TransitionTexture(...) = 0;
    // virtual void TransitionBuffer(...) = 0;

    // ✅ Material 内部处理绑定
    // ✅ 后端内部处理屏障
};

} // namespace NN::Runtime::Render
```

### 3.3 NNRuntimeRenderAssets（资产层）

```
Engine/Source/Experiments/NNRuntimeRenderAssets/
├── CMakeLists.txt
├── Include/
│   ├── NNRAssetsConfig.h
│   │
│   ├── Material/
│   │   ├── NNMaterial.h                ← C# 可见的材质接口
│   │   ├── NNMaterialInstance.h
│   │   └── NNMaterialParam.h
│   │
│   ├── Shader/
│   │   ├── NNShaderAsset.h
│   │   ├── NNShaderVariant.h
│   │   └── NNShaderSource.h
│   │
│   └── Cache/
│       ├── NNPipelineCache.h           ← PSO 缓存（内部）
│       └── NNAssetRegistry.h
│
├── Source/
│   ├── Material/
│   │   ├── NNMaterial.cpp
│   │   ├── NNMaterialInstance.cpp
│   │   └── NNMaterialParam.cpp
│   ├── Shader/
│   │   ├── NNShaderAsset.cpp
│   │   ├── NNShaderVariant.cpp
│   │   └── NNShaderSource.cpp
│   └── Cache/
│       ├── NNPipelineCache.cpp
│       └── NNAssetRegistry.cpp
│
└── Tests/
    └── TestMaterial.cpp
```

```cpp
// NNMaterial.h — C# 通过 Handle 使用，C++ 内部使用 NNRef
namespace NN::Runtime::Assets {

class NNMaterial {
public:
    // C++ 内部 API（使用 NNRef）
    void SetFloat(const std::string& name, float value);
    void SetInt(const std::string& name, int value);
    void SetVector4(const std::string& name, const float4& value);
    void SetTexture(const std::string& name, NNRef<INNTexture> texture);
    void SetSampler(const std::string& name, NNRef<INNSampler> sampler);

    // 渲染时应用（内部处理 PSO + SRB 绑定）
    void Apply(INNCommandList* cmd, NNPipelineCache* cache,
               NNRenderHandle renderPass, const NNVertexLayout& vertexLayout);

    // 获取 Shader
    NNRef<INNShader> GetShader() const;

private:
    NNRef<INNShader> m_Shader;
    std::vector<NNMaterialParam> m_Params;

    // 内部缓存
    NNRef<INNPipelineState> m_CachedPSO;
    // SRB 由后端内部管理
};

} // namespace NN::Runtime::Assets
```

**Material.Apply() 内部流程**：
```
Material.Apply(cmd, cache, renderPass, vertexLayout)
    ↓
cache.GetOrCreatePSO(m_Shader, vertexLayout, renderPass)
    ↓ 创建/获取 PSO（PSO = Shader + VertexLayout + RenderPass + Blend + Rasterizer + DepthStencil）
cmd.SetPipelineState(pso)
    ↓
后端内部: 设置 SRB（纹理/CB/Sampler 绑定）
    ↓
后端内部: 处理资源屏障
    ↓
cmd.Draw(...)
```

**注意**: Material 本身不感知 RenderPass，但渲染器在调用 Apply 时传入当前 RenderPass，由 PipelineCache 负责组合出正确的 PSO。这样：
- C# 层只设置 Shader + 参数（不知道 RenderPass/PSO 存在）
- Renderer 层决定 RenderPass + VertexLayout
- PipelineCache 层缓存 PSO 组合

### 3.4 NNRuntimeRenderBootstrap（引导层）

```
Engine/Source/Experiments/NNRuntimeRenderBootstrap/
├── CMakeLists.txt
├── Include/
│   └── NNRenderBootstrap.h
└── Source/
    └── NNRenderBootstrap.cpp
```

```cpp
// NNRenderBootstrap.h
namespace NN::Runtime::Render {

struct NNRenderDeviceCreateInfo {
    NNRenderBackendType Backend;
    void* Window;
    uint32_t Width;
    uint32_t Height;
    bool EnableValidation;
};

class NNRenderBootstrap {
public:
    static NNRef<INNRenderDevice> CreateDevice(const NNRenderDeviceCreateInfo& info);
    static std::vector>NNRenderBackendType> GetAvailableBackends();
    static const char* GetBackendName(NNRenderBackendType type);
};

} // namespace NN::Runtime::Render
```

### 3.5 NNRuntimeDiligent（后端实现）

```
Engine/Source/Experiments/NNRuntimeDiligent/
├── CMakeLists.txt
├── Include/
│   ├── NNDiligentConfig.h
│   │
│   ├── Device/
│   │   ├── NNDiligentDevice.h          ← 实现 INNRenderDevice
│   │   └── NNDiligentSwapChain.h
│   │
│   ├── Resources/
│   │   ├── NNDiligentBuffer.h
│   │   ├── NNDiligentTexture.h
│   │   └── NNDiligentSampler.h
│   │
│   ├── Pipeline/
│   │   ├── NNDiligentPipelineState.h
│   │   ├── NNDiligentShader.h
│   │   └── NNDiligentInputLayout.h
│   │
│   ├── Command/
│   │   ├── NNDiligentCommandList.h     ← 封装 IDeviceContext
│   │   └── NNDiligentRenderQueue.h
│   │
│   └── Internal/
│       ├── NNDiligentResourceBinding.h ← 内部使用，不暴露
│       ├── NNDiligentShaderCompiler.h
│       └── NNDiligentHelpers.h
│
├── Source/
│   ├── Device/
│   ├── Resources/
│   ├── Pipeline/
│   ├── Command/
│   └── Internal/
│
└── Tests/
    ├── TestDeviceCreation.cpp
    ├── TestBufferCreation.cpp
    ├── TestTextureCreation.cpp
    ├── TestCommandList.cpp
    └── TestTriangleDemo.cpp
```

**关键映射**：
```
Diligent::IDeviceContext  →  NNDiligentCommandList
Diligent::IRenderDevice   →  NNDiligentDevice
Diligent::ISwapChain      →  NNDiligentSwapChain
Diligent::IBuffer         →  NNDiligentBuffer
Diligent::ITexture        →  NNDiligentTexture
Diligent::IShader         →  NNDiligentShader
Diligent::IPipelineState  →  NNDiligentPipelineState
Diligent::IShaderResourceBinding → NNDiligentResourceBinding (内部)
```

---

## 4. NNNativeEngineAPI（C# 边界）

```cpp
// NNNativeEngineAPI.h — 唯一做 Handle ↔ NNRef 转换的地方
extern "C" {

// ===== 设备（EngineContext 持有，不经过 HandleRegistry）=====
NNE_Device* NNE_CreateDevice(const NNE_DeviceCreateInfo* info);
void       NNE_DestroyDevice(NNE_Device* device);

// ===== 资源创建（返回 Handle，内部 NNRef → Register → Handle）=====
NNHandle NNE_CreateTexture(NNE_Device* device, const NNE_TextureDesc* desc, const void* data);
NNHandle NNE_CreateBuffer(NNHandle device, const NNE_BufferDesc* desc, const void* data);
NNHandle NNE_CreateShader(NNHandle device, const NNE_ShaderDesc* desc);
NNHandle NNE_CreateMaterial(NNHandle shader);

// ===== 材质操作（Handle → Registry 查找 → 调用内部 API）=====
void NNE_MaterialSetFloat(NNHandle material, const char* name, float value);
void NNE_MaterialSetTexture(NNHandle material, const char* name, NNHandle texture);
void NNE_MaterialSetSampler(NNHandle material, const char* name, NNHandle sampler);

// ===== 渲染 =====
void NNE_DrawMesh(NNHandle cmd, NNHandle mesh, NNHandle material, const float* transform);

// ===== 资源销毁（Handle → Registry 移除 → 释放 NNRef）=====
void NNE_ReleaseHandle(NNHandle handle);

} // extern "C"
```

**C# 侧**：
```csharp
// C# 只看到 Handle
using TextureHandle = ulong;
using MaterialHandle = ulong;
using MeshHandle = ulong;

public static class NNGraphics
{
    // 设备（EngineContext 持有，C# 拿到的是原生指针封装）
    [DllImport("NNRuntime")]
    public static extern IntPtr NNE_CreateDevice(ref NNE_DeviceCreateInfo info);

    [DllImport("NNRuntime")]
    public static extern void NNE_DestroyDevice(IntPtr device);

    // 资源（返回 Handle）
    [DllImport("NNRuntime")]
    public static extern TextureHandle NNE_CreateTexture(IntPtr device, int w, int h, int fmt);

    [DllImport("NNRuntime")]
    public static extern void NNE_MaterialSetTexture(ulong mat, string name, ulong tex);

    [DllImport("NNRuntime")]
    public static extern void NNE_DrawMesh(IntPtr cmd, ulong mesh, ulong mat, float[] transform);
}

// 使用
IntPtr device = NNGraphics.NNE_CreateDevice(ref createInfo);
TextureHandle albedo = NNGraphics.NNE_CreateTexture(device, 1024, 1024, 1);
MaterialHandle mat = NNGraphics.NNE_CreateMaterial(shader);
NNGraphics.NNE_MaterialSetTexture(mat, "_Albedo", albedo);
NNGraphics.NNE_DrawMesh(cmd, mesh, mat, transformMatrix);
// C# 永远不知道 Diligent / PSO / SRB 的存在
```

---

## 5. 依赖关系（严格单向）

```
NNRuntimeCore
└── (无外部依赖)

NNRuntimeRender
├── NNRuntimeCore (Handle 类型定义)
└── NNCore (HCoreTypes.h)

NNRuntimeRenderAssets
├── NNRuntimeRender (使用接口)
├── NNRuntimeCore (Handle 注册)
└── NNCore

NNRuntimeRenderBootstrap
├── NNRuntimeRender (接口)
├── NNRuntimeDiligent (实现)
└── DiligentCore (条件编译)

NNRuntimeDiligent
├── NNRuntimeRender (实现接口)
├── DiligentCore (仅此模块)
└── NNCore

NNNativeEngineAPI
├── NNRuntimeRender
├── NNRuntimeRenderAssets
├── NNRuntimeCore (Handle 转换)
└── NNRuntimeRenderBootstrap

❌ NNRuntimeRender 不依赖 NNRuntimeDiligent
❌ NNRuntimeRender 不依赖 NNRuntimeRenderAssets
❌ NNRuntimeRenderAssets 不依赖 NNRuntimeDiligent
❌ NNRuntimeCore 不依赖任何渲染模块
```

---

## 6. 实施计划（调整后）

### Phase 1: 核心 + 接口 + 设备（预计 3-4 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 1.1 | 创建 NNRuntimeCore 模块 | NNRef + NNObjectHandleRegistry + NNTypes |
| 1.2 | 创建 NNRuntimeRender 模块 | 所有接口头文件 |
| 1.3 | 创建 NNRuntimeDiligent 模块 | 目录 + CMakeLists.txt |
| 1.4 | 创建 NNRuntimeRenderBootstrap 模块 | CreateDevice |
| 1.5 | 实现 NNDiligentDevice | 后端设备创建 |
| 1.6 | Hello Triangle 验证 | TestTriangleDemo（C++ 直接调用接口）|

**验收标准**: 通过接口层创建 Diligent 后端，渲染彩色三角形。

### Phase 2: 资源层（预计 3-4 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 2.1 | NNDiligentBuffer | VB/IB/CB/SB |
| 2.2 | NNDiligentTexture | 2D/3D/Cube |
| 2.3 | NNDiligentSampler | 采样器 |
| 2.4 | NNDiligentRenderTarget | FBO |
| 2.5 | HandleRegistry 集成 | C++ 对象 ↔ Handle |
| 2.6 | 单元测试 | TestBuffer / TestTexture / TestHandle |

**验收标准**: C++ 内部用 NNRef，Handle 注册正常工作。

### Phase 3: Shader + Pipeline（预计 4-5 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 3.1 | NNDiligentShader | HLSL 编译 |
| 3.2 | NNDiligentInputLayout | 顶点布局 |
| 3.3 | NNDiligentPipelineState | PSO 创建 |
| 3.4 | NNDiligentResourceBinding | SRB（内部）|
| 3.5 | NNDiligentCommandList | 命令录制 |
| 3.6 | NNDiligentRenderQueue | 渲染队列 |
| 3.7 | 集成测试 | 通过 CommandList 渲染带纹理几何体 |

**验收标准**: 完整命令录制管线，Material.Apply() 内部处理绑定。

### Phase 4: Runtime 集成（预计 3-5 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 4.1 | VGFX 适配层 | NNRuntimeRender 接口实现 VGFX |
| 4.2 | Renderer2D 适配 | 2D 渲染器迁移 |
| 4.3 | ImGui 集成 | 编辑器后端切换 |
| 4.4 | NNNativeEngineAPI | C API（Handle ↔ NNRef 转换）|
| 4.5 | C# 验证 | C# 通过 Handle API 渲染 |
| 4.6 | 性能对比 | OpenGL vs Vulkan |

**验收标准**: 现有 Runtime 功能等价运行，C# 通过 Handle 渲染。

---

**到这里合并主干** ✅

---

### Phase 5: Render Graph 实验版（预计 5-7 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 5.1 | NNResourceNode | 资源节点 |
| 5.2 | NNPassNode | Pass 节点 |
| 5.3 | NNRenderGraphBuilder | Builder API |
| 5.4 | 图编译 | 拓扑排序 + 屏障推导 |
| 5.5 | 图执行 | 生成 CommandList 命令 |
| 5.6 | 资源自动管理 | 生命周期追踪 |
| 5.7 | 调试工具 | DOT 导出 |
| 5.8 | 多 Pass 验证 | Shadow + Forward |

**验收标准**: Render Graph 替代手写 Pass 编排。

### Phase 6: Material 系统完善（预计 3-4 天）

| 任务 | 说明 | 产出 |
|------|------|------|
| 6.1 | NNMaterial 完善 | 多 Pass 支持 |
| 6.2 | NNMaterialInstance | 材质实例 |
| 6.3 | NNShaderAsset | Shader 资产加载 |
| 6.4 | NNShaderVariant | 变体管理 |
| 6.5 | NNPipelineCache | PSO 缓存 |
| 6.6 | NNAssetRegistry | 资产注册表 |

**验收标准**: 完整的材质 + Shader 资产系统。

### Phase 7: SRP（远期目标）

| 任务 | 说明 | 产出 |
|------|------|------|
| 7.1 | IRenderPipeline 接口 | C# 可继承的渲染管线 |
| 7.2 | 默认 Forward Pipeline | 默认前向渲染 |
| 7.3 | 默认 Deferred Pipeline | 默认延迟渲染 |
| 7.4 | C# SRP 示例 | C# 自定义渲染流程 |

**验收标准**: C# 可自定义渲染管线。

---

## 7. 技术决策

### 7.1 后端选择
```
Auto → Vulkan > D3D12 > Metal > OpenGL
通过 NNRenderBootstrap::CreateDevice()
```

### 7.2 着色器
- **统一**: HLSL
- **编译**: DiligentCore 内置
- **资产**: NNShaderAsset（后端无关）

### 7.3 内存
- **C++ 内部**: NNRef<T>（强类型）
- **C# 边界**: NNHandle（uint64_t）
- **转换点**: NNNativeEngineAPI（唯一）

### 7.4 线程
```
主线程: 命令录制
加载线程: 资源创建（Device 线程安全）
未来: Deferred Context + INNRenderQueue
```

---

## 8. 验收标准

- [ ] **Phase 1**: 接口 + Diligent 后端渲染三角形
- [ ] **Phase 2**: NNRef 创建资源，Handle 注册正常
- [ ] **Phase 3**: Material.Apply() 内部处理 PSO + SRB
- [ ] **Phase 4**: C# 通过 Handle API 渲染，Runtime 功能等价
- [ ] **Phase 5**: Render Graph 替代手写 Pass
- [ ] **Phase 6**: 完整 Material/Shader 资产系统
- [ ] **Phase 7**: C# SRP 自定义渲染

---

## 9. 风险与对策

| 风险 | 影响 | 对策 |
|------|------|------|
| Diligent 版本升级 | 高 | 封装隔离，类型不泄漏 |
| Handle 版本管理 | 中 | 简单递增，ABA 概率极低 |
| Material.Apply() 性能 | 中 | PipelineCache 缓存 PSO |
| 某些平台后端不成熟 | 中 | 保留 OpenGL 回退 |
| SRP 设计复杂 | 低 | 远期目标，先跑通 Phase 1-4 |

---

## 10. 后续展望

- **自研 Vulkan**: NNRuntimeVulkan 实现同一套接口
- **WebGPU**: NNRuntimeWebGPU
- **SRP**: C# 自定义渲染管线
- **ECS**: Handle 天然适配 Entity-Component
- **网络同步**: Handle 可序列化
- **热重载**: Handle 不变，底层资源重建
- **Ray Tracing**: 硬件加速光线追踪

---

> **下一步**: 审查通过后，开始 Phase 1 实施。
