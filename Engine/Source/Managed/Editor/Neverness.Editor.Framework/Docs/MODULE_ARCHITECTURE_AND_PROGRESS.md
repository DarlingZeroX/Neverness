# Neverness.Editor.Framework — 编辑器基础设施层

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Editor.Framework` |
| **命名空间** | `Neverness.Editor.Framework` |
| **层级** | Level 0 — 不引用任何其他 Editor 模块 |
| **职责** | 纯基础设施：Docking、Panel、Menu、Command、Toolbar |
| **不负责** | ContentBrowser、SceneBrowser、Viewport、Console、Selection（已迁至 Feature 模块） |

上级文档：[MANAGED_EDITOR_ARCHITECTURE_AND_PROGRESS.md](../../MANAGED_EDITOR_ARCHITECTURE_AND_PROGRESS.md)

## 2. 目录结构

```
Neverness.Editor.Framework/
├── Public/                                     namespace .Public
│   ├── Module.cs                               # EditorFrameworkModule 静态入口
│   ├── EditorCommand.cs                        # 命令模型
│   ├── EditorMenuItem.cs                       # 菜单项数据结构
│   ├── EditorMenuRegistry.cs                   # 菜单注册静态门面
│   ├── IMenuContributor.cs                     # 主菜单扩展接口
│   ├── IContextMenuContributor.cs              # 上下文菜单扩展接口
│   └── IconsFontAwesome5Pro.cs                 # 图标常量
│
├── Interface/                                  namespace .Interface
│   └── UIInterface.cs                          # IPanel / IEditorPanel / IPanelManager
│                                               # ICommandRegistry / IMenuRegistry / IContextMenuRegistry
│
├── Private/                                    namespace .Private
│   ├── ModuleImp.cs                            # 基础设施初始化（仅注册 Toolbar / MainWindow / MenuBar）
│   ├── EditorShell.cs                          # 编辑器壳
│   ├── PanelManager.cs                         # 面板管理器（实现 IPanelManager）
│   ├── CommandRegistry.cs                      # 命令注册表（实现 ICommandRegistry）
│   ├── IEditorCommand.cs                       # 命令接口
│   ├── DockingLayout.cs                        # 停靠布局
│   │
│   ├── Menu/                                   namespace .Private.Menu
│   │   ├── MenuRegistryImp.cs                  # 实现 IMenuRegistry
│   │   ├── ContextMenuManager.cs               # 实现 IContextMenuRegistry
│   │   ├── MenuTree.cs / MenuTreeBuilder.cs
│   │   ├── ImGuiMenuRenderer.cs
│   │   ├── ImGuiToolbarRenderer.cs
│   │   ├── DynamicMenuProvider.cs
│   │   ├── ShortcutFormatter.cs
│   │   └── ToolbarCommand.cs
│   │
│   └── Panel/                                  namespace .Private.Panel
│       ├── EditorPanel.cs                      # 面板抽象基类
│       └── Main/
│           ├── EditorMainWindow.cs              # 主窗口（DockSpace 宿主）
│           └── EditorMenuBar.cs                 # 菜单栏
```

## 3. 关键接口（从具体类提取）

Framework 通过 `Interface/UIInterface.cs` 暴露以下抽象接口，供 Core 及 Feature 模块消费：

| 接口 | 实现类 | 说明 |
|------|--------|------|
| `IPanelManager` | `PanelManager` | 面板注册/查找/AddChildPanel |
| `ICommandRegistry` | `CommandRegistry` | 命令注册/执行 |
| `IMenuRegistry` | `MenuRegistryImp` | 主菜单项注册 |
| `IContextMenuRegistry` | `ContextMenuManager` | 上下文菜单注册/渲染 |
| `IEditorPanel` | 各 Feature 面板 | 面板 OnGUI 接口 |

## 4. 依赖

- `Neverness.Editor.ImGui` / `Neverness.Editor.ImGuiEx`（ThirdParty）
- `Neverness.Editor.ProjectSystem`
- `Neverness.Runtime.Application`（Window / EngineTime）

**不引用：** 任何其他 Editor 模块（Core / Assets / Scene / Inspector 等）

## 5. 变更记录

| 日期 | 说明 |
|------|------|
| **2026-05-15** | Phase 5 首包 Shell |
| **2026-05-19** | 纳入 Neverness Editor 总文档 |
| **2026-05-23** | 架构重构：Feature 代码迁出至 Core / Assets / Scene；提取 IPanelManager 等接口；Framework 纯基础设施化 |
