# Neverness.Runtime.Host — UCO 导出层

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Host` |
| **命名空间** | `Neverness.Managed.Runtime` |
| **职责** | 向 Native 导出 `[UnmanagedCallersOnly]`：`Bootstrap`、`GetApiVersion`、`RuntimeTick` |
| **不负责** | 启动逻辑（见 **Neverness.Runtime.Bootstrap**）、Interop（见 **Neverness.Runtime.Interop**）、CoreCLR（Legacy 见 **NNRuntimeManagedHostLegacy**） |

> **命名说明**：本程序集历史上称「Host」，现仅表示 **Native 可调用的 UCO 入口**，**不是** 架构上的 Runtime 宿主。主路径为进程内嵌 CLR + `RuntimeBootstrap`。

## 2. UCO 接口

| 方法 | 说明 |
|------|------|
| `Bootstrap(nint)` | 安装 API 表并初始化；非阻塞 |
| `GetApiVersion()` | `(ApiVersion << 16) \| LayoutVersion` |
| `RuntimeTick(float)` | 托管 Kernel 单帧 |

## 3. 依赖

- 仅引用 `Neverness.Runtime.Bootstrap`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-19** | Migration-4：瘦身为 UCO 转发层；不再聚合 Gameplay/Scene 等产品程序集 |
