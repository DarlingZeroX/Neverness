# Editor 模块目录

> `Engine/Source/Managed/Editor/` — 编辑器端 C# 模块，依赖 Runtime 层。

## 模块清单

| 模块 | 程序集 | 说明 |
|------|--------|------|
| [NevernessEditor](#nevernesseditor) | `NevernessEditor.csproj` | 编辑器入口（Program/Main + Native API 加载） |
| [Neverness.Editor.Framework](#nevernesseditorframework) | `Neverness.Editor.Framework.csproj` | 编辑器核心框架（菜单系统、ContentBrowser、面板管理、Docking） |
| [Neverness.Editor.Serialization](#nevernesseditorserialization) | `Neverness.Editor.Serialization.csproj` | 场景 / 资产 JSON 序列化与再水合 |
| [Neverness.Editor.Reflection](#nevernesseditorreflection) | `Neverness.Editor.Reflection.csproj` | 类型 / 字段元数据反射注册表 |
| [Neverness.Editor.Inspector](#nevernesseditorinspector) | `Neverness.Editor.Inspector.csproj` | 属性检查器 UI（PropertyDrawer） |
| [Neverness.Editor.UndoRedo](#nevernesseditorundoredo) | `Neverness.Editor.UndoRedo.csproj` | 撤销 / 重做栈 |
| [Neverness.Editor.ImGuiEx](#nevernesseditorimguiex) | `Neverness.Editor.ImGuiEx.csproj` | ImGui 扩展工具 |
| [Neverness.Editor.ProjectSystem](#nevernesseditorprojectsystem) | `Neverness.Editor.ProjectSystem.csproj` | 项目路径（Assets/Intermediate/Settings） |
| [Neverness.Editor.Assets](#nevernesseditorassets) | `Neverness.Editor.Assets.csproj` | 资产创建工厂系统（Registry + Factory + 反射自动发现） |
| [ThirdParty](#thirdparty) | — | 第三方依赖（ImGui / ImGuiNodeEditor / ImGuizmo / HexaGen） |

---

### NevernessEditor

编辑器可执行入口。

| 文件 | 说明 |
|------|------|
| `Program.cs` | Main 入口 |
| `EditorApplicationRunner.cs` | 编辑器应用启动/循环驱动 |
| `EditorLaunchOptions.cs` | 启动参数 |
| `NativeApiTableLoader.cs` | Native API 表加载 |

---

### Neverness.Editor.Framework

编辑器核心框架：菜单系统、ContentBrowser、面板管理、Docking 布局。

**Public API**

| 文件 | 说明 |
|------|------|
| `Public/EditorCommand.cs` | 命令定义（Id / DisplayName / Execute / CanExecute） |
| `Public/EditorMenuItem.cs` | 菜单项（Path / Command / Icon / SortOrder） |
| `Public/EditorMenuRegistry.cs` | 命令 / 贡献者注册入口 |
| `Public/IContextMenuContributor.cs` | 上下文菜单贡献者接口 |
| `Public/IMenuContributor.cs` | 主菜单贡献者接口 |
| `Public/ContentBrowserContextMenu.cs` | ContentBrowser 上下文菜单公开 API |
| `Public/IconsFontAwesome5Pro.cs` | FontAwesome 图标常量 |
| `Public/Module.cs` | 编辑器模块安装入口 |

**Core**

| 文件 | 说明 |
|------|------|
| `Private/Core/ContentBrowser.cs` | 内容浏览器核心逻辑（目录导航 / 创建目录 / 删除 / 重命名） |
| `Private/Core/ContentBrowserItem.cs` | ContentItem / ContentFile / ContentDirectory 层级 |

**Menu System**

| 文件 | 说明 |
|------|------|
| `Private/Menu/ContextMenuManager.cs` | 上下文菜单管理器（注册 / 运行时上下文 / 渲染） |
| `Private/Menu/BuiltinMenuContributor.cs` | 内置主菜单贡献（File / Edit / View / Help） |
| `Private/Menu/ContentBrowserContextMenuContributor.cs` | ContentBrowser 右键菜单贡献（基础操作；资产创建由 Assets 模块接管） |
| `Private/Menu/MenuRegistryImp.cs` | 菜单注册表内部实现 |
| `Private/Menu/MenuTree.cs` | 菜单树结构 |
| `Private/Menu/MenuTreeBuilder.cs` | 菜单树构建器 |
| `Private/Menu/ImGuiMenuRenderer.cs` | ImGui 菜单渲染 |
| `Private/Menu/ImGuiToolbarRenderer.cs` | ImGui 工具栏渲染 |
| `Private/Menu/DynamicMenuProvider.cs` | 动态菜单提供者 |
| `Private/Menu/ShortcutFormatter.cs` | 快捷键格式化 |
| `Private/Menu/ToolbarCommand.cs` | 工具栏命令 |

**Panel System**

| 文件 | 说明 |
|------|------|
| `Private/Panel/Main/EditorMainWindow.cs` | 主窗口（Viewport + 菜单栏 + 工具栏） |
| `Private/Panel/Main/EditorMenuBar.cs` | 主菜单栏 |
| `Private/Panel/ContentBrowser/ContentBrowserPanel.cs` | ContentBrowser 面板（ImGui 布局 + 交互） |
| `Private/Panel/ContentBrowser/ContentBrowserFileUIBox.cs` | 文件缩略图 UI 盒 |
| `Private/Panel/EditorViewport.cs` | 视口面板 |
| `Private/Panel/EditorPanel.cs` | 面板基类 |
| `Private/Panel/ConsolePanel.cs` | 控制台面板 |
| `Private/Panel/SceneBrowser.cs` | 场景浏览器面板 |

**Infrastructure**

| 文件 | 说明 |
|------|------|
| `Private/ModuleImp.cs` | 模块安装实现（初始化 ContentBrowser / 注册贡献者 / 面板） |
| `Private/PanelManager.cs` | 面板管理器 |
| `Private/EditorShell.cs` | 编辑器 Shell |
| `Private/DockingLayout.cs` | Docking 布局 |
| `Private/SelectionService.cs` | 选择服务 |
| `Private/CommandRegistry.cs` | 命令注册表（IEditorCommand 体系） |
| `Private/IEditorCommand.cs` | 旧版命令接口 |

**Interface**

| 文件 | 说明 |
|------|------|
| `Interface/UIInterface.cs` | UI 接口定义 |

---

### Neverness.Editor.Serialization

场景与资产的 JSON 序列化 / 反序列化。

| 文件 | 说明 |
|------|------|
| `SceneSerializer.cs` | 场景 DTO（SceneDocument / SceneEntityEntry）JSON 序列化 |
| `SceneRehydrator.cs` | 场景 DTO → Native 实体再水合（经 SceneManager） |
| `AssetSerializer.cs` | 资产元数据 JSON 序列化 |
| `VersionTolerance.cs` | JSON 序列化选项（camelCase / 缩进 / 版本兼容） |

---

### Neverness.Editor.Reflection

类型与字段元数据反射注册表，供 Inspector 和序列化使用。

| 文件 | 说明 |
|------|------|
| `ReflectionRegistry.cs` | 类型元数据注册表（缓存 TypeMetadata） |
| `TypeMetadata.cs` | 类型元数据（可序列化属性列表） |
| `PropertyMetadata.cs` | 属性元数据（名称 / 类型 / 读写器） |
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

### Neverness.Editor.Assets

资产创建工厂系统——UE AssetTools 风格，Registry + Reflection + Factory 模式。
Editor-only 模块，Runtime 无感知。工厂通过程序集扫描自动注册，支持未来插件扩展和 Hot Reload。

| 文件 | 说明 |
|------|------|
| `AssetFactories/IAssetFactory.cs` | 资产工厂接口（DisplayName / Category / Icon / FileExtension / CreateAsset） |
| `AssetFactories/AssetFactoryRegistry.cs` | 工厂注册表（单例，程序集自动发现，支持 Rediscover） |
| `AssetFactories/SceneAssetFactory.cs` | 场景工厂（从 ContentBrowser.CreateNewSceneFile 迁移） |
| `AssetFactories/MaterialAssetFactory.cs` | 材质工厂（JSON 占位格式） |
| `AssetFactories/LuaScriptAssetFactory.cs` | Lua 脚本工厂 |
| `AssetActions/AssetTypeActions.cs` | 资产类型操作元数据 |
| `AssetActions/AssetTypeActionsRegistry.cs` | 资产类型操作注册表 |
| `AssetCreationMenuContributor.cs` | ContentBrowser 右键 "Create >" 子菜单贡献者（动态生成） |
| `Public/Module.cs` | 模块安装入口（AssetsModule.Install） |
| `Private/AssetsModuleImp.cs` | 内部安装实现（自动发现 + 元数据注册 + 贡献者注册） |

---

### ThirdParty

第三方依赖（不直接修改）。

| 目录 | 说明 |
|------|------|
| `Neverness.Editor.ImGui` | ImGui C# 绑定 |
| `Neverness.Editor.ImGuiNodeEditor` | ImGui Node Editor |
| `Neverness.Editor.ImGuizmo` | ImGuizmo（Gizmo 操作） |
| `HexaGen.Runtime` | HexaGen 运行时 |
