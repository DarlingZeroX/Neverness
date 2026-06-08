# RmlDiligent 性能优化实施计划

**日期**：2026-06-08
**目标**：优化 Diligent 后端性能，对齐 DX12/GL3 官方后端的 benchmark FPS
**范围**：仅 RmlDiligent 模块

---

## 一、性能瓶颈分析

### 当前架构 vs 官方后端

| 特性 | GL3 | DX12 | Diligent（当前） |
|------|-----|------|-----------------|
| Geometry Buffer | 每 geometry 独立 VBO/IBO (GL_STATIC_DRAW) | BufferMemoryManager 子分配池 | **每 geometry 独立 IMMUTABLE buffer** ← 瓶颈 |
| PSO 切换 | `active_program` 脏检测 | 每次都 SetPipelineState | **每次都 SetPipelineState** ← 瓶颈 |
| CB 上传 | glUniform 按需更新 | 持久映射 + memcpy | **每次 Map/Unmap** ← 瓶颈 |
| VB/IB 绑定 | 每次 glBindVertexArray | 每次绑定 VB/IB View | **RenderGeometry + DrawIndexedGeometry 双重绑定** ← 瓶颈 |

### 瓶颈优先级

| 优先级 | 瓶颈 | 位置 | 影响 |
|--------|------|------|------|
| **P0** | CompileGeometry 每次 CreateBuffer(IMMUTABLE) | RenderInterface.cpp:1420 | D3D12 CreateBuffer 是最重操作，每帧数百次 |
| **P1** | 每次 SetPipelineState（无脏检测） | RenderInterface.cpp:1528 | 冗余驱动开销 |
| **P1** | 每次 Map/Unmap ConstantBuffer | RenderInterface.cpp:1531 | 每 draw call 一次 CPU→GPU 拷贝 |
| **P1** | DrawIndexedGeometry 冗余 VB/IB 绑定 | RenderInterface.cpp:1038 | RenderGeometry 已绑定，又绑定一次 |
| **P2** | WriteDefaultFullscreenVertices 重复写入 | RenderInterface.cpp:2143 | 每全屏 pass 重写相同 4 个顶点 |
| **P2** | BoxShadowDiagLog 文件 I/O | RenderInterface.cpp:234 | 前 30 帧 fopen/fprintf/fclose |

---

## 二、实施步骤

### Phase 1：PSO 脏检测（P1，最简单）

**原理**：GL3 用 `active_program` 缓存当前 program ID，只在变化时调用 `glUseProgram`。Diligent 当前每次都调用 `SetPipelineState`。

**修改文件**：`Private/RmlDiligentRenderInterface.h`, `Private/RmlDiligentRenderInterface.cpp`

**方案**：
```cpp
// 头文件添加成员
Diligent::IPipelineState* m_ActivePSO = nullptr;

// RenderGeometry 中添加脏检测
if (pso != m_ActivePSO) {
    m_Context->SetPipelineState(pso);
    m_ActivePSO = pso;
}
```

**注意**：
- `SetPipelineState` 在 Diligent 中是轻量操作（只设置指针），但减少冗余调用仍有意义
- 需要在 `BeginFrame` 时重置 `m_ActivePSO = nullptr`
- filter/composite 路径也需要跟踪

---

### Phase 2：去除 DrawIndexedGeometry 冗余 VB/IB 绑定（P1，简单）

**原理**：`RenderGeometry` 已经绑定了 VB/IB，`DrawIndexedGeometry` 又绑定了一次。

**修改文件**：`Private/RmlDiligentRenderInterface.cpp`

**方案**：
- `DrawIndexedGeometry` 不再绑定 VB/IB，只负责 scissor + draw
- 或者将 VB/IB 绑定移到 `DrawIndexedGeometry` 中，`RenderGeometry` 不绑定

**选择**：将 VB/IB 绑定统一到 `DrawIndexedGeometry`，`RenderGeometry` 只设置 PSO/CB/SRB。

---

### Phase 3：BufferMemoryManager 子分配池（P0，最复杂）

**原理**：DX12 后端使用 `BufferMemoryManager` + `D3D12MA::VirtualBlock` 从大的持久映射 buffer 中子分配 VB/IB/CB，避免每帧 CreateBuffer。

**修改文件**：新建 `Renderer/RmlDiligentBufferManager.h`, `Renderer/RmlDiligentBufferManager.cpp`，修改 `Private/RmlDiligentRenderInterface.cpp`

**方案**：

#### 3.1 BufferManager 设计

```cpp
class BufferManager {
public:
    struct Allocation {
        Diligent::IBuffer* buffer;    // 共享的大 buffer
        Uint64 offset;                // 子分配偏移
        Uint64 size;                  // 分配大小
        void* mappedPtr;              // 持久映射指针（CB 用）
    };

    void Initialize(Diligent::IRenderDevice* device, Uint64 vbSize, Uint64 ibSize);

    // VB 子分配（USAGE_DYNAMIC，持久映射）
    Allocation AllocVertex(const void* data, Uint64 size);
    // IB 子分配（USAGE_DYNAMIC，持久映射）
    Allocation AllocIndex(const void* data, Uint64 size);

    // 每帧重置（ring buffer 模式）
    void FrameReset();

private:
    // 大 buffer（持久映射）
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_VB;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_IB;
    void* m_VBMapped = nullptr;
    void* m_IBMapped = nullptr;

    // 简单的线性分配器（ring buffer）
    Uint64 m_VBOffset = 0;
    Uint64 m_IBOffset = 0;
    Uint64 m_VBSize = 0;
    Uint64 m_IBSize = 0;
};
```

#### 3.2 GeometryHandle 修改

```cpp
struct GeometryHandle {
    // 旧：独立 buffer
    // Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;
    // Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;

    // 新：子分配引用
    BufferManager::Allocation vbAlloc;
    BufferManager::Allocation ibAlloc;
    Uint32 indexCount = 0;
};
```

#### 3.3 CompileGeometry 修改

```cpp
Rml::CompiledGeometryHandle CompileGeometry(...) {
    auto* handle = new GeometryHandle();
    handle->vbAlloc = m_BufferManager.AllocVertex(vertices.data(), vertices.size() * sizeof(Rml::Vertex));
    handle->ibAlloc = m_BufferManager.AllocIndex(indices.data(), indices.size() * sizeof(int));
    handle->indexCount = static_cast<Uint32>(indices.size());
    return reinterpret_cast<Rml::CompiledGeometryHandle>(handle);
}
```

#### 3.4 RenderGeometry 修改

```cpp
void RenderGeometry(...) {
    // ... PSO/CB/SRB 设置 ...

    // 使用子分配的 VB/IB（带 offset）
    Uint64 vbOffset = geom->vbAlloc.offset;
    Diligent::IBuffer* pVB = geom->vbAlloc.buffer;
    m_Context->SetVertexBuffers(0, 1, &pVB, &vbOffset, ...);
    m_Context->SetIndexBuffer(geom->ibAlloc.buffer, 0, ...);

    // Draw
    Diligent::DrawIndexedAttribs drawAttrs(geom->indexCount, ...);
    m_Context->DrawIndexed(drawAttrs);
}
```

#### 3.5 ReleaseGeometry 修改

```cpp
void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    // 子分配模式下，buffer 归 BufferManager 管理
    // 只需 delete handle，buffer 在 FrameReset 时统一回收
    delete reinterpret_cast<GeometryHandle*>(geometry);
}
```

#### 3.6 FrameReset

在 `BeginFrame` 中调用 `m_BufferManager.FrameReset()`，重置线性分配器。

**注意**：
- VB/IB 大小需要足够大（建议 4MB VB + 1MB IB 作为初始值）
- 如果一帧用完了，需要 grow（重新分配更大的 buffer）
- Ring buffer 模式下，每帧从头分配，上一帧的 allocation 在 FrameReset 时失效
- RmlUi 的 CompileGeometry/ReleaseGeometry 生命周期是跨帧的，需要特殊处理

**关键问题**：RmlUi 的 geometry 生命周期是跨帧的（CompileGeometry 在元素首次渲染时调用，ReleaseGeometry 在元素销毁时调用）。这意味着不能简单地每帧重置 buffer。

**解决方案**：使用 **双缓冲 + 延迟释放**：
1. 每帧从当前 buffer 的 offset 处分配
2. FrameReset 时切换到另一个 buffer（双缓冲）
3. 释放操作只标记，等 buffer 切换后才真正回收

或者更简单：使用 **USAGE_DYNAMIC + 每帧 Map/Discard** 模式，但复用 buffer 对象而不是每帧创建新的。

实际上，最简单的方案是：**CompileGeometry 时创建 USAGE_DYNAMIC buffer，ReleaseGeometry 时延迟到下一帧释放**。这避免了 CreateBuffer 的开销（复用 buffer 对象），同时保持了 RmlUi 的跨帧生命周期语义。

但更彻底的方案是参考 DX12 的 BufferMemoryManager，使用虚拟内存子分配。不过这需要 D3D12MA 或类似的支持，在 Diligent 中可以用 `USAGE_DYNAMIC` + `MAP_WRITE` + 大 buffer + offset 来模拟。

---

### Phase 4：CB 批量上传（P1，中等）

**原理**：DX12 后端使用持久映射的大 CB buffer + memcpy，避免每帧 Map/Unmap。

**修改文件**：`Private/RmlDiligentRenderInterface.h`, `Private/RmlDiligentRenderInterface.cpp`

**方案**：

#### 4.1 CB Pool 设计

```cpp
// 每帧的 CB 分配器
struct CBAllocator {
    Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;  // 大 CB buffer
    void* mappedPtr = nullptr;                           // 持久映射
    Uint64 offset = 0;                                   // 当前分配偏移
    Uint64 size = 0;                                     // 总大小

    void Initialize(Diligent::IRenderDevice* device, Uint64 totalSize);
    void* Alloc(Uint64 allocSize, Uint64 alignment = 256);
    void Reset() { offset = 0; }
};
```

#### 4.2 RenderGeometry 修改

```cpp
// 旧：每次 Map/Unmap
Diligent::MapHelper<MainCB> cb(m_Context, m_ConstantBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
UploadTransformToCB(cb, m_Transform, translation);

// 新：从 pool 分配
void* cbPtr = m_CBAllocator.Alloc(sizeof(MainCB));
MainCB* cb = reinterpret_cast<MainCB*>(cbPtr);
UploadTransformToCB(cb, m_Transform, translation);
// 绑定 CB（使用 offset）
```

**注意**：Diligent 的 `SetConstantBuffer` 支持 offset 参数，可以绑定到大 buffer 的指定偏移。

---

### Phase 5：去除 WriteDefaultFullscreenVertices 重复写入（P2，简单）

**原理**：全屏四边形的默认顶点数据（4 个顶点）永远不变，不需要每次重写。

**修改方案**：
- 创建一个 `m_FullscreenVBDefault`（IMMUTABLE），存储默认顶点
- `DrawFullscreenPassthrough` 使用默认 VB，不调用 `WriteDefaultFullscreenVertices`
- 只有需要自定义 UV 时（如 DropShadow 的 UV 偏移）才写入 dynamic VB

---

### Phase 6：去除 BoxShadowDiagLog 文件 I/O（P2，简单）

**原理**：前 30 帧的诊断日志写入文件有 fopen/fprintf/fclose 开销。

**修改方案**：
- 用 `std::string` 或 `std::vector<char>` 内存 buffer 替代文件 I/O
- 或者用 `#ifdef` 条件编译完全禁用

---

## 三、实施顺序

1. **Phase 1**：PSO 脏检测（10 分钟，立即见效）
2. **Phase 2**：去除冗余 VB/IB 绑定（10 分钟）
3. **Phase 5**：去除 WriteDefaultFullscreenVertices 重复写入（10 分钟）
4. **Phase 6**：去除 BoxShadowDiagLog 文件 I/O（5 分钟）
5. **Phase 4**：CB 批量上传（1 小时，需要改 CB 绑定逻辑）
6. **Phase 3**：BufferMemoryManager 子分配池（2-3 小时，最复杂）

建议先做 Phase 1+2+5+6（简单优化），测试效果后再做 Phase 3+4（架构优化）。

---

## 四、预期效果

| Phase | 预期 FPS 提升 | 原因 |
|-------|-------------|------|
| Phase 1 | 5-10% | 减少冗余 SetPipelineState 调用 |
| Phase 2 | 5-10% | 减少冗余 VB/IB 绑定 |
| Phase 3 | 30-50% | 消除 CreateBuffer/ReleaseBuffer 开销（最大提升） |
| Phase 4 | 10-15% | 消除 Map/Unmap 开销 |
| Phase 5 | 2-5% | 减少 VB 写入 |
| Phase 6 | 1-3% | 消除文件 I/O |

总预期：**50-80% FPS 提升**（主要来自 Phase 3）

---

## 五、修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Private/RmlDiligentRenderInterface.h` | 添加 m_ActivePSO、CBAllocator、BufferManager 成员 |
| `Private/RmlDiligentRenderInterface.cpp` | PSO 脏检测、VB/IB 绑定优化、CB 批量上传、CompileGeometry 重构、fullscreen VB 优化、diag log 优化 |
| `Renderer/RmlDiligentBufferManager.h` | 新建：VB/IB 子分配池 |
| `Renderer/RmlDiligentBufferManager.cpp` | 新建：子分配池实现 |
