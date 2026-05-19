# Neverness.Editor.Framework — 编辑器壳层

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Editor.Framework` |
| **命名空间** | `VisionGal.Managed.Editor`（待统一为 `Neverness.Managed.Editor`） |
| **职责** | **EditorShell**、**DockingLayout**、**EditorPanel**、**CommandRegistry**、**SelectionService** |
| **不负责** | Runtime 启动与 Kernel（见 **Neverness.Runtime.Bootstrap**） |

上级文档：[MANAGED_EDITOR_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_EDITOR_ARCHITECTURE_AND_PROGRESS.md)

## 2. 主要能力

| 类型 | 说明 |
|------|------|
| `EditorShell` | 编辑器根壳 |
| `EditorPanel` | 面板基类（Console、Hierarchy 等） |
| `IEditorCommand` / `CommandRegistry` | 命令模式 |
| `SelectionService` | 选区服务 |

## 3. 依赖

- `Neverness.Runtime.Object`（及 Reflection 等 Runtime 地基）

## 4. 路线图

| 阶段 | 内容 | 状态 |
|------|------|------|
| **E-1** | 壳层巩固 | **进行中** |
| **E-2** | Inspector / Graph Editor | **未开始** |
| **E-3** | Asset Browser、Scene Editor | **未开始** |

## 5. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | Phase 5 首包 Shell |
| **2026-05-19** | 纳入 Neverness Editor 总文档；与 Runtime Host 主路径解耦 |
