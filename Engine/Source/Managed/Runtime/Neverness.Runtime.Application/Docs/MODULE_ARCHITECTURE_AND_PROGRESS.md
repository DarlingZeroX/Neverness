# Neverness.Runtime.Application

## 职责

封装 `NNApplicationAPI`，提供 `ApplicationHost` 静态门面：

- `Initialize` / `OpenWindow` / `PumpEvents` / `Shutdown`

## 不负责

- Gameplay、Scene、RuntimeLoop、Editor UI

## 依赖

- `Neverness.Runtime.Engine`（ABI 镜像）
- `Neverness.Runtime.Interop`（`EngineNativeApiBootstrap`）

## 进度

- [x] Phase 4：`ApplicationHost` 门面
- [x] 产品代码禁止直接调用 `delegate*`
