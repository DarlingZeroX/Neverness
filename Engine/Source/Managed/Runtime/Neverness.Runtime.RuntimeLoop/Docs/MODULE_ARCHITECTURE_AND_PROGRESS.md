# Neverness.Runtime.RuntimeLoop — 托管 Kernel 主循环

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.RuntimeLoop` |
| **命名空间** | `Neverness.Managed.RuntimeLoop` |
| **职责** | 与 Native `RuntimeScheduler` 对称的帧管线 |
| **不负责** | Engine ABI、产品 Gameplay 逻辑 |

## 2. 管线顺序

`EarlyUpdate → FixedUpdate(0..N) → Update → LateUpdate → MainThreadDispatcher.Drain → Render`

## 3. 公开 API

| 类型 | 说明 |
|------|------|
| `RuntimeLoop` | 组合调度器 |
| `FrameScheduler` | Fixed 累加与步数上限 |
| `SubsystemScheduler` | 子系统分桶 Tick |
| `MainThreadDispatcher` | 主线程委托队列 |
| `ManagedRuntimeScheduler` | **已弃用**；薄包装 |

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | P0-1 首包 |
| **2026-05-19** | M-5：`RuntimeLoop` 协作类型重构 |
