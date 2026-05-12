# VGEditorExtensions 模块架构与进展

## 1. 定位

提供 **`ISequenceEditorExtension`** 与 **`SequenceExtensionRegistry`**，用于在 **编译期/静态** 注册扩展（Inspector、Validator、Graph 装饰等将按 Phase 8 后续子任务逐步加槽位）。

**不**在本阶段实现 DLL 热插拔。

## 2. API

| 类型 | 说明 |
|------|------|
| `ISequenceEditorExtension` | `GetExtensionId`、`OnEditorSessionBegin/End` |
| `SequenceExtensionRegistry` | `Register`、`NotifySessionBegin/End` |

宿主在 `InitializeChrome` 后调用 `NotifySessionBegin`，销毁前 `NotifySessionEnd`。

## 3. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-12 | Phase 8 首批：接口 + 注册表实现 + 本文档。 |
