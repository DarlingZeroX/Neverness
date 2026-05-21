# Neverness.Runtime.Application

## 职责

封装 Runtime Host 子表，提供静态门面：

| 类型 | 门面 | Native 子表 |
|------|------|-------------|
| 生命周期 | `ApplicationHost` | `NNApplicationAPI` — `Initialize` / `PumpEvents` / `Shutdown` / `BeginFrame` / `EndFrame` |
| 窗口 | `WindowHost` | `NNWindowAPI` — 创建/销毁、几何、状态、`GetNativeHandle` |

## 不负责

- Gameplay、Scene、RuntimeLoop、Editor UI 业务逻辑

## 依赖

- `Neverness.Runtime.Engine`（ABI 镜像、`NNWindowHandle`）
- `Neverness.Runtime.Interop`（`EngineNativeApiBootstrap`）

## 消费者

- **NervernessEditor 首包**：`ApplicationHost.Initialize` → `WindowHost.Create` → 事件泵与帧循环

## 进度

- [x] Phase 4：`ApplicationHost` 门面
- [x] **layout v7**：`WindowHost` + Application 与 Window 解耦
- [x] 产品代码禁止直接调用 `delegate*`
