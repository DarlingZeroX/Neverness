# RmlDiligent 性能优化实施计划（v3）

**日期**：2026-06-08
**目标**：基于实测数据定位瓶颈，对齐 DX12/GL3 官方后端 benchmark FPS
**范围**：仅 RmlDiligent 模块
**原则**：先 Profile，再优化。不猜瓶颈。

---

## 一、核心结论

### Diligent 后端没有性能问题。

经过完整的 Phase A 性能分析，确认：

| 阶段 | 耗时 | 说明 |
|------|------|------|
| RmlUi CPU（context->Render 内部） | ~775ms/帧 | DOM 遍历、样式计算、布局、几何生成 |
| Diligent 后端 GPU 调用 | ~20ms/帧 | PSO/SRB/Draw/Map 等，完全正常 |
| CompileGeometry（启动阶段） | ~820ms 一次性 | 481 次 CreateBuffer(IMMUTABLE)，之后不再调用 |

**20 倍 FPS 差距（0.2 vs 4 FPS）的根因是 RmlUi 核心 CPU 开销，不是 Diligent 后端。**

---

## 二、已完成工作

### 2.1 功能修复（非性能优化）

| 修复 | 类型 | 状态 |
|------|------|------|
| 矩阵顺序 | 功能 Bug | ✅ |
| Projection | 功能 Bug | ✅ |
| D3D NDC Z 范围 | 功能 Bug | ✅ |
| SRB 兼容性 | 功能 Bug | ✅ |
| Stencil | 功能 Bug | ✅ |
| Transform | 功能 Bug | ✅ |
| 去除冗余 VB/IB 绑定 | 代码清理 | ✅ |
| 禁用 diag log | 代码清理 | ✅ |

### 2.2 Phase A：性能计数器系统

**目标**：统计每帧各 API 调用次数和耗时，用数据定位瓶颈。

**实现**：`#ifdef RML_PERF_COUNTERS` 条件编译，零开销。

**计数器清单**：

| 计数器 | 含义 | 数据 |
|--------|------|------|
| CompileGeometry | CompileGeometry 调用次数 | 481（只在启动时） |
| ReleaseGeometry | ReleaseGeometry 调用次数 | 0 |
| CreateBuffer | CreateBuffer 调用次数 | 962（只在启动时） |
| RenderGeometry | RenderGeometry 调用次数 | 490/帧 |
| SetPSO (real/req) | PSO 切换（实际/请求） | 102/491 |
| CommitSRB (real/req) | SRB 绑定（实际/请求） | 149/491 |
| DrawIndexed | DrawIndexed 调用次数 | 491/帧 |
| MapCB | ConstantBuffer Map 次数 | 490/帧 |

**帧计时**：

| 阶段 | 耗时 |
|------|------|
| BeginFrame | 0.15ms |
| EndFrame | 0.15ms |
| Backend GPU 调用总计 | ~20ms |
| context->Render() | ~775ms |

**每类 GPU 操作耗时分解**：

| 操作 | 耗时 |
|------|------|
| BindPSO | 1.5ms |
| BindSRB | 0.7ms |
| MapCB | 5.6ms |
| DrawIndexed | 7.2ms |
| SetScissor | 1.5ms |
| SetVB+IB | 1.4ms |
| SrbLookup | 1.4ms |
| **sum** | **~20ms** |

### 2.3 RenderStateCache（PSO/SRB 脏检测）

**实现**：统一的 `BindPSO()` / `BindSRB()` 入口，指针比较脏检测。

**效果**：

| 指标 | 优化前 | 优化后 | 减少 |
|------|--------|--------|------|
| SetPSO | 491 | 102 | 79% |
| CommitSRB | 491 | 149 | 70% |

**设计要点**：
- `BindPSO` 变化时自动清空 `currentSRB`（PSO 的 implicit signature 不同）
- `BindSRB` 保留 mode 参数（传参模式），缓存比较只看 srb 指针
- 所有 20 个 `SetPipelineState` 调用点统一走 `BindPSO()`
- 所有 20 个 `CommitShaderResources` 调用点统一走 `BindSRB()`

---

## 三、性能分析时间线

### 3.1 初始假设（已推翻）

| 假设 | 结论 |
|------|------|
| CompileGeometry 每帧重建 Buffer | ❌ 只在启动时调用 481 次 |
| PSO/SRB 冗余切换是主要瓶颈 | ⚠️ 有优化（79%/70%），但不是 20 倍差距的原因 |
| CB Map/Unmap 是瓶颈 | ❌ 只有 5.6ms |
| D3D12MA 能提升 30-50% | ❌ 无证据支持 |
| D3D12 Validation 慢 | ❌ 未确认 |

### 3.2 最终定位

```
Total = ~900ms/帧
├── context->Render() = ~775ms（RmlUi 核心 CPU）
├── Backend GPU 调用 = ~20ms（Diligent 后端，正常）
├── BeginFrame+EndFrame = 0.3ms
└── CompileGeometry = ~820ms（一次性启动开销）
```

**RmlUi 核心 CPU 开销（775ms）是唯一的真正瓶颈。**

---

## 四、benchmark 测试条件

**路径**：`Engine/Source/ThirdParty/RmlUi/Samples/basic/benchmark/`

**默认模式**：
- `run_update = true`：每帧调用 `performance_test()`，生成 50 行复杂 RML，调用 `SetInnerRML()` 重建 DOM
- `run_loop = true`：每帧渲染

**DOM 复杂度**：每行包含 button、link、input[range]、select、嵌套 div

**FPS 计算**：200 帧滑动窗口平均，包含启动阶段开销

---

## 五、修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Public/RmlDiligentRenderInterface.h` | RenderStateCache、PerfCounters、BindPSO/BindSRB 声明、Mark* 方法 |
| `Private/RmlDiligentRenderInterface.cpp` | BindPSO/BindSRB 实现、20 个调用点替换、计时系统、输出格式 |
| `Renderer/RmlDiligentProgramId.h` | 新增 TextureStencilEqual（之前完成） |
| `Plan/RmlDiligent_Performance_Optimization.md` | 本文档 |

---

## 六、下一步方向

### 如果要优化 RmlUi CPU 侧（775ms）

1. RmlUi 核心 profiling（不是后端），定位 `context->Render()` 内部热点
2. 可能的方向：
   - 样式缓存优化
   - 布局计算增量更新
   - 几何生成批处理
   - DOM diffing（避免每帧全量重建）

### 如果要继续优化 Diligent 后端（已到位）

- RenderStateCache 已实现（PSO 79% 减少，SRB 70% 减少）
- 后端 GPU 调用 20ms/帧对 490 draw 来说完全正常
- 无进一步优化必要

### D3D12MA（暂不需要）

- CreateBuffer 只在启动时调用，不是每帧瓶颈
- 在证明 Buffer 创建是每帧热点之前，不引入 D3D12MA

---

## 七、关键经验教训

1. **先 Profile，再优化**：猜瓶颈 → 测数据 → 推翻假设 → 迭代
2. **看起来像性能问题的，往往是功能 Bug**：矩阵、Projection、Z 范围、SRB、Stencil、Transform
3. **计数器要覆盖所有路径**：CompileGeometry 的计数器只统计了后端调用，没覆盖 RmlUi 内部路径
4. **VS Profiler 采样可能落在启动阶段**：不能直接用采样比例推断每帧开销
5. **帧计时要包住整个主循环**：只测后端会漏掉 RmlUi CPU 开销
