# Editor 模块目录

> `Engine/Source/Managed/Editor/` — 编辑器端 C# 模块，依赖 Runtime 层。

## 模块清单

| 模块 | 程序集 | 层级 | 说明 |
|------|--------|------|------|
| [NevernessEditor](#nevernesseditor) | `NevernessEditor` | L4 | 编辑器入口（Program/Main + Native API 加载） |
| [Neverness.Editor.Framework](#nevernesseditorframework) | `Neverness.Editor.Framework` | L0 | 纯基础设施（Docking / Menu / Panel / Command） |
| [Neverness.Editor.Core](#nevernesseditorcore) | `Neverness.Editor.Core` | L1 | 运行时中枢（上下文 / 服务总线 / 事件 / 生命周期） |
| [Neverness.Editor.Assets](#nevernesseditorassets) | `Neverness.Editor.Assets` | L2 | ContentBrowser / 资产工厂 / 上下文菜单 |
| [Neverness.Editor.Scene](#nevernesseditorscene) | `Neverness.Editor.Scene` | L3 | 场景浏览器 / 视口 / 场景编辑 |
| [Neverness.Editor.Serialization](#nevernesseditorserialization) | `Neverness.Editor.Serialization` | L1 | 场景 / 资产 JSON 序列化与再水合 |
| [Neverness.Editor.Reflection](#nevernesseditorreflection) | `Neverness.Editor.Reflection` | L1 | 类型 / 字段元数据反射注册表 |
| [Neverness.Editor.Inspector](#nevernesseditorinspector) | `Neverness.Editor.Inspector` | L2 | 属性检查器 UI（PropertyDrawer） |
| [Neverness.Editor.UndoRedo](#nevernesseditorundoredo) | `Neverness.Editor.UndoRedo` | L1 | 撤销 / 重做栈 |
| [Neverness.Editor.ImGuiEx](#nevernesseditorimguiex) | `Neverness.Editor.ImGuiEx` | L0 | ImGui 扩展工具 |
| [Neverness.Editor.ProjectSystem](#nevernesseditorprojectsystem) | `Neverness.Editor.ProjectSystem` | L0 | 项目路径（Assets/Intermediate/Settings） |
| [ThirdParty](#thirdparty) | — | — | 第三方依赖（ImGui / ImGuiNodeEditor / ImGuizmo / HexaGen） |

---

### NevernessEditor

编辑器可执行入口（Level 4 — 引用所有模块）。

| 文件 | 说明 |
|------|------|
| `Program.cs` | Main 入口 |
| `EditorApplicationRunner.cs` | 编辑器应用启动/循环驱动（Install 顺序：Framework → Core → Assets → Scene） |
| `EditorLaunchOptions.cs` | 启动参数 |
| `NativeApiTableLoader.cs` | Native API 表加载 |

---

### Neverness.Editor.Framework

编辑器纯基础设施（Level 0 — 不引用任何其他 Editor 模块）。

| 文件 | 说明 |
|------|------|
| `Public/Module.cs` | EditorFrameworkModule 静态入口 |
| `Public/EditorCommand.cs` | 命令定义 |
| `Public/EditorMenuItem.cs` | 菜单项数据结构 |
| `Public/EditorMenuRegistry.cs` | 菜单注册静态门面 |
| `Public/IMenuContributor.cs` | 主菜单贡献者接口 |
| `Public/IContextMenuContributor.cs` | 上下文菜单贡献者接口 |
| `Public/IconsFontAwesome5Pro.cs` | FontAwesome 图标常量 |
| `Interface/UIInterface.cs` | IPanel / IEditorPanel / IPanelManager / ICommandRegistry / IMenuRegistry / IContextMenuRegistry |
| `Private/ModuleImp.cs` | 基础设施初始化（仅注册 Toolbar / MainWindow / MenuBar） |
| `Private/PanelManager.cs` | 面板管理器（实现 IPanelManager） |
| `Private/CommandRegistry.cs` | 命令注册表（实现 ICommandRegistry） |
| `Private/EditorShell.cs` | 编辑器壳 |
| `Private/DockingLayout.cs` | 停靠布局 |
| `Private/IEditorCommand.cs` | 命令接口 |
| `Private/Menu/ContextMenuManager.cs` | 上下文菜单管理器（实现 IContextMenuRegistry） |
| `Private/Menu/MenuRegistryImp.cs` | 菜单注册表实现（实现 IMenuRegistry） |
| `Private/Menu/MenuTree.cs` | 菜单树结构 |
| `Private/Menu/MenuTreeBuilder.cs` | 菜单树构建器 |
| `Private/Menu/ImGuiMenuRenderer.cs` | ImGui 菜单渲染 |
| `Private/Menu/ImGuiToolbarRenderer.cs` | ImGui 工具栏渲染 |
| `Private/Menu/DynamicMenuProvider.cs` | 动态菜单提供者 |
| `Private/Menu/ShortcutFormatter.cs` | 快捷键格式化 |
| `Private/Menu/ToolbarCommand.cs` | 工具栏命令 |
| `Private/Panel/EditorPanel.cs` | 面板抽象基类 |
| `Private/Panel/Main/EditorMainWindow.cs` | 主窗口（DockSpace 宿主） |
| `Private/Panel/Main/EditorMenuBar.cs` | 菜单栏 |

---

### Neverness.Editor.Core

编辑器运行时中枢（Level 1 — 仅引用 Framework + UndoRedo）。

| 文件 | 说明 |
|------|------|
| `Public/Module.cs` | EditorCoreModule 静态入口 |
| `Public/IEditorFeature.cs` | Feature 注册接口（插件化核心） |
| `Public/IEditorContext.cs` | 编辑器上下文接口（Panels / Commands / Menus / Events / State / 服务定位器） |
| `Public/IEditorEventBus.cs` | 事件总线接口 + EditorEventType 枚举 |
| `Public/EditorState.cs` | 编辑器状态（PlayMode / ScenePath / SelectedEntity） |
| `Public/IContentBrowserService.cs` | ContentBrowser 服务接口（供 Scene/Inspector 跨模块消费） |
| `Private/CoreModuleImp.cs` | Core 初始化（创建 EditorContext / LifecycleManager / 自动发现 Features） |
| `Private/EditorContext.cs` | IEditorContext 实现（聚合 Framework 接口 + 服务字典） |
| `Private/EditorEventBus.cs` | IEditorEventBus 实现（同步 + 延迟分发） |
| `Private/ModuleLifecycleManager.cs` | 模块生命周期管理（Kahn 拓扑排序初始化） |
| `Private/SelectionService.cs` | 选中服务 |
| `Private/BuiltinMenuContributor.cs` | 内置主菜单贡献（File / Edit / View / Help） |
| `Private/Panel/ConsolePanel.cs` | 控制台面板 |

---

### Neverness.Editor.Assets

资产模块（Level 2 — 引用 Framework + Core）。

| 文件 | 说明 |
|------|------|
| `Public/Module.cs` | AssetsModule 静态入口 |
| `Private/AssetsModuleImp.cs` | 内部安装实现（ContentBrowser 引擎 / 面板 / 上下文菜单 / 资产工厂） |
| `Private/Core/ContentBrowser.cs` | 内容浏览器核心逻辑（目录导航 / 创建 / 删除 / 重命名） |
| `Private/Core/ContentBrowserItem.cs` | ContentItem / ContentFile / ContentDirectory 层级 |
| `Private/Core/ContentBrowserService.cs` | IContentBrowserService 实现（包装 ContentBrowser 单例） |
| `Private/Panel/ContentBrowserPanel.cs` | ContentBrowser 面板（ImGui 布局 + 交互） |
| `Private/Panel/ContentBrowserFileUIBox.cs` | 文件缩略图 UI 盒 |
| `Private/Context/ContentBrowserContextMenu.cs` | ContentBrowser 上下文菜单公开 API |
| `Private/Context/ContentBrowserContextMenuContributor.cs` | ContentBrowser 右键菜单贡献（基础操作） |
| `AssetCreationMenuContributor.cs` | "Create >" 子菜单贡献者（动态生成） |
| `AssetFactories/IAssetFactory.cs` | 资产工厂接口 |
| `AssetFactories/AssetFactoryRegistry.cs` | 工厂注册表（程序集自动发现） |
| `AssetFactories/SceneAssetFactory.cs` | 场景工厂 |
| `AssetFactories/MaterialAssetFactory.cs` | 材质工厂 |
| `AssetFactories/LuaScriptAssetFactory.cs` | Lua 脚本工厂 |
| `AssetActions/AssetTypeActions.cs` | 资产类型操作元数据 |
| `AssetActions/AssetTypeActionsRegistry.cs` | 资产类型操作注册表 |

---

### Neverness.Editor.Scene

场景编辑模块（Level 3 — 引用 Framework + Core）。

| 文件 | 说明 |
|------|------|
| `Public/Module.cs` | SceneModule 静态入口 |
| `Private/SceneModuleImp.cs` | 场景模块初始化（注册 SceneBrowser / EditorViewport） |
| `Private/Panel/SceneBrowser.cs` | 场景浏览器面板 |
| `Private/Panel/EditorViewport.cs` | 视口面板 |

---

### Neverness.Editor.Serialization

场景与资产的 JSON 序列化 / 反序列化。

| 文件 | 说明 |
|------|------|
| `SceneSerializer.cs` | 场景 DTO JSON 序列化 |
| `SceneRehydrator.cs` | 场景 DTO → Native 实体再水合 |
| `AssetSerializer.cs` | 资产元数据 JSON 序列化 |
| `VersionTolerance.cs` | JSON 序列化选项（camelCase / 缩进 / 版本兼容） |

---

### Neverness.Editor.Reflection

类型与字段元数据反射注册表。

| 文件 | 说明 |
|------|------|
| `ReflectionRegistry.cs` | 类型元数据注册表 |
| `TypeMetadata.cs` | 类型元数据 |
| `PropertyMetadata.cs` | 属性元数据 |
| `ReflectionAttributes.cs` | 反射特性定义 |

---

### Neverness.Editor.Inspector

属性检查器 UI 渲染。

| 文件 | 说明 |
|------|------|
| `InspectorView.cs` | 检查器视图 |
| `PropertyDrawer.cs` | 属性绘制器 |

---

### Neverness.Editor.UndoRedo

撤销 / 重做基础设施。

| 文件 | 说明 |
|------|------|
| `UndoStack.cs` | 撤销栈 |
| `IUndoableCommand.cs` | 可撤销命令接口 |
| `PropertyChangeCommand.cs` | 属性变更命令 |

---

### Neverness.Editor.ImGuiEx

ImGui 扩展工具。

| 文件 | 说明 |
|------|------|
| `Public/ImGuiEx.cs` | ImGui 扩展方法 |

---

### Neverness.Editor.ProjectSystem

项目路径定义。

| 文件 | 说明 |
|------|------|
| `Public/Module.cs` | `ProjectPaths`：Assets / Intermediate / Settings / EngineResource 路径常量 |

---

### ThirdParty

第三方依赖（不直接修改）。

| 目录 | 说明 |
|------|------|
| `Neverness.Editor.ImGui` | ImGui C# 绑定 |
| `Neverness.Editor.ImGuiNodeEditor` | ImGui Node Editor |
| `Neverness.Editor.ImGuizmo` | ImGuizmo（Gizmo 操作） |
| `HexaGen.Runtime` | HexaGen 运行时 |
