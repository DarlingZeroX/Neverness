# Neverness.Runtime.Assets — 托管资产 GUID 与导入

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Assets` |
| **命名空间** | `Neverness.Managed.Assets` |
| **职责** | `AssetDatabase`、`ImportPipeline`；可选经 Interop 同步 Native `AssetRegistry` |
| **不负责** | GPU 上传、Pak 解压（Native Kernel） |

## 2. 依赖

- `Neverness.Runtime.Object`、`Engine`、`Interop`

## 3. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | Phase 5 地基 |
