# Neverness Managed Editor — 架构与总进度

本文档描述 **Neverness 编辑器** 托管侧架构。编辑器与 **Runtime** 并列，同属 `Engine/Source/Managed`，但职责为 **工具链与 UI**，不替代 Runtime Kernel。

上级文档：[MANAGED_ARCHITECTURE_AND_PROGRESS.md](../MANAGED_ARCHITECTURE_AND_PROGRESS.md)

---

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 编辑器 UI 壳层、面板系统、命令系统、资产工具、场景编辑 |
| **不负责** | CoreCLR 启动、Native Kernel、Gameplay 运行时主循环（属 Runtime） |
| **与 Runtime 关系** | 引用 Runtime 模块（如 `Neverness.Runtime.Application`）；不以 `Neverness.Runtime.Host` 为编辑器入口 |

---

## 2. 模块层级

```
Level 4:  NevernessEditor (Exe)              ← 入口，引用所有模块

Level 3:  Neverness.Editor.Scene             ← 场景浏览器 / 视口 / Gizmo

Level 2:  Neverness.Editor.Assets            ← ContentBrowser / 资产工厂
          Neverness.Editor.Inspector         ← 属性检查器

Level 1:  Neverness.Editor.Core              ← 运行时中枢 / 服务总线 / 生命周期
          Neverness.Editor.Reflection        ← 反射工具
          Neverness.Editor.UndoRedo          ← 撤销重做
          Neverness.Editor.Serialization     ← 序列化桥接

Level 0:  Neverness.Editor.Framework         ← 纯基础设施（Docking / Menu / Panel）
          Neverness.Editor.ImGui             ← ThirdParty
          Neverness.Editor.ImGuiEx           ← ImGui 扩展
          Neverness.Editor.ProjectSystem     ← 项目路径 / VFS
```

**依赖规则：** Framework ← Core ← Feature。Feature 之间可有选择性依赖（Scene → Assets），但禁止反向。

---

## 3. 程序集结构

```
Engine/Source/Managed/Editor/
├── NevernessEditor/                            ← Exe 入口
├── Neverness.Editor.Framework/                 ← Level 0：纯基础设施
├── Neverness.Editor.Core/                      ← Level 1：运行时中枢
├── Neverness.Editor.Assets/                    ← Level 2：ContentBrowser + 资产工厂
├── Neverness.Editor.Scene/                     ← Level 3：场景编辑
├── Neverness.Editor.Inspector/                 ← Level 2：属性检查
├── Neverness.Editor.Reflection/                ← Level 1：反射工具
├── Neverness.Editor.UndoRedo/                  ← Level 1：撤销重做
├── Neverness.Editor.Serialization/             ← Level 1：序列化桥接
├── Neverness.Editor.ProjectSystem/             ← Level 0：项目路径
├── Neverness.Editor.ImGuiEx/                   ← Level 0：ImGui 扩展
├── Docs/                                       ← 架构文档
└── ThirdParty/
    ├── Neverness.Editor.ImGui/
    ├── Neverness.Editor.ImGuiNodeEditor/
    ├── Neverness.Editor.ImGuizmo/
    └── HexaGen.Runtime/
```

---

## 4. 启动流程

```csharp
// EditorApplicationRunner.Run()
EditorFrameworkModule.Install(window);   // 1. 基础设施（PanelManager / Menu / Command / Docking）
EditorCoreModule.Install();              // 2. 运行时中枢（EditorContext / EventBus / LifecycleManager）
AssetsModule.Install();                  // 3. 资产模块（ContentBrowser / 资产工厂 / 上下文菜单）
SceneModule.Install();                   // 4. 场景模块（SceneBrowser / EditorViewport）
```

---

## 5. 模块文档

| 模块 | 文档 |
|------|------|
| **Neverness.Editor.Framework** | [Framework/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md](Neverness.Editor.Framework/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) |
| **Neverness.Editor.Serialization** | [Serialization/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md](Neverness.Editor.Serialization/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) |

---

## 6. 路线图

| 代号 | 方向 | 状态 |
|------|------|------|
| **E-1** | Framework 壳层巩固（Dock、Panel、Command） | **已完成** |
| **E-1.5** | 架构重构：Core / Assets / Scene 模块化 | **已完成** |
| **E-2** | Inspector / Graph Editor 与 Runtime.Reflection 对接 | **未开始** |
| **E-3** | 高级场景编辑（Gizmo、层级树、拖放） | **未开始** |

---

## 7. 变更记录

| 日期 | 说明 |
|------|------|
| **2026-05-19** | 初版：Neverness 品牌、Editor 分支总文档 |
| **2026-05-23** | 架构重构完成：Core / Assets / Scene 模块化；Framework 纯化；更新层级图与启动流程 |
