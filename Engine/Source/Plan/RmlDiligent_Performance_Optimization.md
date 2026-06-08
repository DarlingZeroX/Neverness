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
| VB/IB 绑定 | 每次 glBindVertexArray | 每次绑定 VB/IB View | ~~双重绑定~~ → 已修复 |

### 瓶颈优先级

| 优先级 | 瓶颈 | 状态 | 影响 |
|--------|------|------|------|
| **P0** | CompileGeometry 每次 CreateBuffer(IMMUTABLE) | ❌ 未实施 | D3D12 CreateBuffer 是最重操作，每帧数百次 |
| **P1** | 每次 SetPipelineState（无脏检测） | ⚠️ 已回退（见下方） | 冗余驱动开销 |
| **P1** | 每次 Map/Unmap ConstantBuffer | ❌ 未实施 | 每 draw call 一次 CPU→GPU 拷贝 |
| **P1** | DrawIndexedGeometry 冗余 VB/IB 绑定 | ✅ 已完成 | RenderGeometry 不再重复绑定 |
| **P2** | WriteDefaultFullscreenVertices 重复写入 | ❌ 未实施 | 每全屏 pass 重写相同 4 个顶点 |
| **P2** | BoxShadowDiagLog 文件 I/O | ✅ 已完成 | `#ifdef RML_DIAG_LOG` 条件编译禁用 |

---

## 二、实施状态

### ✅ 已完成

#### Phase 2：去除 DrawIndexedGeometry 冗余 VB/IB 绑定
- `RenderGeometry` 中移除了重复的 `SetVertexBuffers`/`SetIndexBuffer` 调用
- VB/IB 绑定统一在 `DrawIndexedGeometry` 中执行
- **改动**：`Private/RmlDiligentRenderInterface.cpp`（-4 行）

#### Phase 6：禁用 BoxShadowDiagLog 文件 I/O
- `BoxShadowDiagLog` 用 `#ifdef RML_DIAG_LOG` 条件编译禁用
- 取消注释 `#define RML_DIAG_LOG` 可重新启用
- **改动**：`Private/RmlDiligentRenderInterface.cpp`

#### SRB 兼容性修复（附带修复）
- 新增 `m_SRB_Color_StencilEqual`：为 `m_PSO_Color_StencilEqual` 创建独立 SRB
- 新增 `ProgramId::TextureStencilEqual`：纹理 SRB 缓存按 PSO 变体分桶
- **原因**：Diligent 的 implicit signature 不同的 PSO 不能共用 SRB
- **改动**：`Public/RmlDiligentRenderInterface.h`、`Private/RmlDiligentRenderInterface.cpp`、`Renderer/RmlDiligentProgramId.h`

---

### ⚠️ 已回退

#### Phase 1：PSO 脏检测
- **实现**：添加 `m_ActivePSO` 缓存，在 RenderGeometry/DrawColorGeometry/RenderShader 中检查 `pso != m_ActivePSO`
- **问题**：`DrawFullscreenPassthrough` 内部调用 `SetPipelineState` 但不更新 `m_ActivePSO`，导致脏检测状态与 GPU 实际状态不同步。当 filter/composite 路径通过 `DrawFullscreenPassthrough` 切换 PSO 后，`m_ActivePSO` 指向上一个 PSO，后续 `CommitShaderResources` 使用不匹配的 SRB → D3D12 validation error
- **根本原因**：`DrawFullscreenPassthrough` 是通用函数，被 filter chain、composite、EndFrame 等多处调用，无法简单地在其中更新 `m_ActivePSO`（因为调用者不知道 PSO 被改了）
- **正确方案**：需要将 PSO 脏检测扩展到所有 `SetPipelineState` 调用点（包括 `DrawFullscreenPassthrough`），或者使用更精确的跟踪机制（如 `std::unordered_set<IPipelineState*>` 记录已绑定的 PSO）
- **回退**：已完全移除 `m_ActivePSO` 成员和所有脏检测代码

---

### ❌ 未实施

#### Phase 3：BufferMemoryManager 子分配池（P0，最大提升）
- **原理**：DX12 后端用 `BufferMemoryManager` + `D3D12MA::VirtualBlock` 从大 buffer 子分配
- **难点**：RmlUi 的 geometry 生命周期是跨帧的（CompileGeometry → 多帧渲染 → ReleaseGeometry），不能简单每帧重置
- **方案**：USAGE_DYNAMIC 大 buffer + chunk-based 线性分配 + BeginFrame/EndFrame 映射/取消映射
- **曾尝试实施但因 SRB 兼容性问题一并回退**，代码逻辑已验证可行

#### Phase 4：CB 批量上传（P1）
- **原理**：持久映射大 CB buffer + memcpy，避免每帧 Map/Unmap
- **依赖 Phase 3** 的 BufferManager 基础设施

#### Phase 5：去除 WriteDefaultFullscreenVertices 重复写入（P2）
- **原理**：默认全屏四边形顶点不变，用 IMMUTABLE buffer
- **影响较小**，暂不实施

---

## 三、下一步计划

### 优先级 1：重新实施 Phase 3（BufferMemoryManager）

上次实施时因 SRB 问题一并回退，但 BufferManager 代码逻辑已验证可行（MapBuffer/UnmapBuffer API 已修正）。需要：
1. 重新创建 `Renderer/RmlDiligentBufferManager.h/cpp`
2. 修改 `GeometryHandle` 使用 `BufferAllocation`
3. 修改 `CompileGeometry` 使用 `BufferManager::AllocVertex/AllocIndex`
4. 修改 `DrawIndexedGeometry` 使用共享 buffer + offset
5. `BeginFrame` 调用 `m_BufferManager.BeginFrame()`（映射 + 重置偏移）
6. `EndFrame` 调用 `m_BufferManager.EndFrame()`（取消映射）

### 优先级 2：重新实施 Phase 1（PSO 脏检测）

需要解决 `DrawFullscreenPassthrough` 的隐式 PSO 切换问题：
- **方案 A**：在 `DrawFullscreenPassthrough` 中也更新 `m_ActivePSO`
- **方案 B**：不在 `DrawFullscreenPassthrough` 中调用 `SetPipelineState`，改为由调用者设置
- **方案 C**：放弃全局脏检测，只在高频路径（RenderGeometry）做局部脏检测

### 优先级 3：Phase 4（CB 批量上传）

依赖 Phase 3 的 BufferManager，可以在 Phase 3 完成后实施。

---

## 四、预期效果

| Phase | 预期 FPS 提升 | 原因 |
|-------|-------------|------|
| Phase 2 | 5-10% | 减少冗余 VB/IB 绑定 |
| Phase 3 | 30-50% | 消除 CreateBuffer/ReleaseBuffer 开销（最大提升） |
| Phase 1 | 5-10% | 减少冗余 SetPipelineState 调用 |
| Phase 4 | 10-15% | 消除 Map/Unmap 开销 |
| Phase 6 | 1-3% | 消除文件 I/O |

总预期：**50-80% FPS 提升**（主要来自 Phase 3）

---

## 五、修改文件清单

| 文件 | 已完成修改 |
|------|-----------|
| `Private/RmlDiligentRenderInterface.cpp` | 去除冗余 VB/IB 绑定、禁用 diag log、SRB 兼容性修复 |
| `Public/RmlDiligentRenderInterface.h` | 新增 m_SRB_Color_StencilEqual |
| `Renderer/RmlDiligentProgramId.h` | 新增 TextureStencilEqual |
