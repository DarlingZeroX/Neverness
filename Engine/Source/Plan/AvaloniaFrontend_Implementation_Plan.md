# Neverness.Editor.AvaloniaFrontend 实施计划（修订版）

> 原版评分 6/10，第一轮修订 8.5/10，最终版 8.8/10
> 修订依据：用户反馈 10 点实战经验 + 4 点开工前修正

---

## Context

### 背景
当前编辑器使用 ImGui 作为 UI 后端（`Neverness.Editor.ImGuiFrontend`），已通过 MVVM 架构（IViewFactory + ViewModel + Controller + View）实现了 View 层的可替换性。现在需要创建 Avalonia 后端，实现 UE 风格的现代编辑器界面。

### 目标
- 创建 `Neverness.Editor.AvaloniaFrontend` 模块，替代 ImGuiFrontend 作为主编辑器
- 支持主题切换（Dark/Light，后续扩展）
- 使用 Dock 库实现可拖拽停靠的面板布局
- Viewport 使用 NativeControlHost 嵌入原生渲染窗口
- **保留 ImGuiFrontend 作为 Developer Tools Frontend**（GPU Debug、Texture Viewer、Render Graph 等）

### 架构决策
1. **分阶段替换**：AvaloniaFrontend 与 ImGuiFrontend 共存，通过启动参数切换
2. **ImGuiFrontend 不删除**：降级为 Developer Tools Frontend，保留 GPU Debug/Texture Viewer 等工具
3. **Viewport 方案**：Phase 1 使用 NativeControlHost，后续抽象 IViewportSurface
4. **Dock 库**：使用 wieslawsoltes/Dock 12.0（Dock.Avalonia 12.0.0.2 + Dock.Model.Mvvm 12.0.0.2），**Dock 只做容器，不参与业务**
5. **FrontendServices 层**：Core 只看接口（IDockService、IWindowService 等），未来可换 UI 框架
6. **TargetFramework**：统一升级到 net10.0
7. **Avalonia 版本**：12.0.4（最新稳定版）

---

## 现有架构分析

### 依赖关系
```
Neverness.Editor.Framework (UI 无关基础设施)
    ├── IView, IViewModel, IController, PanelViewBase
    ├── IEditorPanel, IPanelManager, PanelManager
    ├── EditorText (i18n), EditorIcons, EditorResourceCache
    └── IMenuRegistry, ICommandRegistry

Neverness.Editor.Core (ViewModel + Controller + CompositionRoot)
    ├── ViewModels: SceneBrowser, Inspector, Viewport, ContentBrowser, Console
    ├── Controllers: SceneBrowser, Inspector, Viewport, ContentBrowser
    ├── EditorCompositionRoot (组装层)
    └── IViewFactory (接口，由 Frontend 实现)

Neverness.Editor.ImGuiFrontend (当前 ImGui View 实现，保留为 Developer Tools)
    ├── ImGuiViewFactory : IViewFactory
    ├── Views: *ImGuiView (继承 PanelViewBase)
    ├── Inspectors: *Inspector (ImGui 实现)
    ├── Shell: ImGuiMainWindow, ImGuiMenuBar
    ├── Menu: ImGuiMenuRenderer
    └── DragDrop: ImGuiAssetDragDropService
```

### 关键接口
- `IViewFactory`：创建 View 并绑定 ViewModel，由 `EditorCompositionRoot.RegisterViewFactory()` 注入
- `IView`：Bind/Unbind/Render，ImGui 版本每帧调用 Render()，Avalonia 版本使用数据绑定
- `IEditorPanel`：OnGUI/OnUpdate/OnFixedUpdate，ImGui 版本在 OnGUI 中渲染，Avalonia 版本为空操作
- `PanelViewBase`：桥接 IView 和 IEditorPanel，Avalonia 版本需要新的基类

### 启动流程
```
EditorApplicationRunner.Run()
  → EditorFrameworkModule.Install()
  → ImGuiFrontendModule.InstallShell(window)  // ← 需要替换
  → EditorCoreModule.Install()
  → ImGuiFrontendModule.Install()              // ← 需要替换
  → ... 其他模块 ...
  → EditorCompositionRoot.Build()              // ← 通过 IViewFactory 创建 View
  → ImGuiFrontendModule.RegisterContextMenuContributors() // ← 需要替换
```

---

## 核心设计原则

### 原则 1：Dock 不侵入 Core ViewModel
```
正确：
  Core ViewModel → Avalonia View → DockControl（容器）

错误：
  Core ViewModel → Dock PanelViewModel → DockControl
```
Dock 只是布局容器，不参与业务数据流。

### 原则 2：NativeWindowHandle 不进 ViewModel
```
正确：
  ViewportView → ViewportHostService → NativeControlHost → HWND

错误：
  EditorViewportViewModel { IntPtr NativeWindowHandle; }
```
ViewModel 保持平台无关，HWND 属于 UI 层。

### 原则 3：FrontendServices 抽象层
```
Core 只看接口：
  IDockService, IWindowService, INotificationService

AvaloniaFrontend 实现：
  AvaloniaDockService, AvaloniaWindowService, AvaloniaNotificationService
```
未来可换 MAUI、Web 等前端。

### 原则 4：先硬编码，后抽象
- Inspector：先硬编码 Transform/Camera/Sprite，组件多了再做 PropertyGrid
- Theme：先 Dark/Light，稳定后再加 Blue/Classic
- Localization：UI 稳定后再做

---

## 实施计划

### Phase 0：项目脚手架 + FrontendServices 接口 ✅ 已完成

**目标**：创建 AvaloniaFrontend 模块的基本项目结构，并在 Framework 中落地 FrontendServices 接口

**设计约束**：
- FrontendServices 接口属于基础设施，必须最早落地
- Core 只看接口（IDockService、IWindowService、INotificationService），不依赖 Avalonia
- AvaloniaFrontend 实现这些接口
- 避免 Phase 1-7 写完后再抽接口导致返工

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Framework/Public/Services/IDockService.cs`
   - Dock 布局服务接口
   - 保存/恢复布局、添加/移除面板

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Framework/Public/Services/IWindowService.cs`
   - 窗口服务接口
   - 创建独立窗口、对话框

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Framework/Public/Services/INotificationService.cs`
   - 通知服务接口
   - 显示通知、模态对话框

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Neverness.Editor.AvaloniaFrontend.csproj`
   ```xml
   <Project Sdk="Microsoft.NET.Sdk">
     <PropertyGroup>
       <TargetFramework>net10.0</TargetFramework>
       <Nullable>enable</Nullable>
       <AvaloniaUseCompiledBindingsByDefault>true</AvaloniaUseCompiledBindingsByDefault>
       <RootNamespace>Neverness.Editor.AvaloniaFrontend</RootNamespace>
     </PropertyGroup>
     <ItemGroup>
       <PackageReference Include="Avalonia" Version="12.0.4" />
       <PackageReference Include="Avalonia.Desktop" Version="12.0.4" />
       <PackageReference Include="Avalonia.Themes.Fluent" Version="12.0.4" />
       <PackageReference Include="Avalonia.Fonts.Inter" Version="12.0.4" />
       <PackageReference Include="Dock.Avalonia" Version="12.0.0.2" />
       <PackageReference Include="Dock.Model.Mvvm" Version="12.0.0.2" />
       <PackageReference Include="Dock.Avalonia.Themes.Fluent" Version="12.0.0.2" />
     </ItemGroup>
     <ItemGroup>
       <ProjectReference Include="..\Neverness.Editor.Framework\Neverness.Editor.Framework.csproj" />
       <ProjectReference Include="..\Neverness.Editor.Core\Neverness.Editor.Core.csproj" />
     </ItemGroup>
   </Project>
   ```

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Public/AvaloniaFrontendModule.cs`
   - 参考 `ImGuiFrontendModule` 的结构
   - `InstallShell()` 启动 Avalonia 应用
   - `Install()` 注册 AvaloniaViewFactory、扫描 Inspector、注册服务
   - `RegisterContextMenuContributors()` 注册上下文菜单

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Public/AvaloniaViewFactory.cs`
   - 实现 `IViewFactory` 接口
   - 创建 Avalonia 版本的 View（先用占位符）

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/App.axaml`
   - Avalonia 应用入口，FluentTheme + Dark 主题
   - 参考 Launcher 的 App.axaml 结构

5. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/App.axaml.cs`
   - Avalonia 应用初始化

6. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Program.cs`
   - Avalonia 应用启动入口

7. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/AvaloniaDockService.cs`
   - 实现 IDockService

8. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/AvaloniaWindowService.cs`
   - 实现 IWindowService

9. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/AvaloniaNotificationService.cs`
   - 实现 INotificationService

**依赖关系**：无

**验证方式**：模块能编译通过，Avalonia 窗口能启动（空白窗口），FrontendServices 接口落地

---

### Phase 1：Dock 布局基础设施 ✅ 已完成（简化版）

**目标**：集成 Dock 库，实现默认编辑器布局

**实施说明**：
- Dock 11.3.2 API 与文档中的 11.2.0 有差异，IDockable 接口体系有变化
- 已实现最小化 Dock 集成（EditorDockFactory + EditorDockLayout + MainEditorWindow）
- 完整布局（SceneBrowser/Viewport/Inspector/ContentBrowser/Console 五区域）需要进一步适配 Dock 11.3.2 API
- Dock theme（FluentDockTheme.axaml）资源路径需确认

**设计约束**：
- Dock 只做容器，不参与业务数据流
- 不创建 Dock PanelViewModel 桥接层
- Core ViewModel 直接绑定到 Avalonia View，View 放入 DockControl

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Dock/EditorDockFactory.cs`
   - 继承 `Factory`（Dock.Model.Mvvm）
   - 创建默认布局：SceneBrowser(左) + Viewport(中) + Inspector(右) + ContentBrowser(底) + Console(底Tab)
   - 定义面板类型映射

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Dock/EditorDockLayout.cs`
   - 布局序列化/反序列化
   - 默认布局定义
   - 布局保存/恢复逻辑

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/MainEditorWindow.axaml`
   - 主编辑器窗口
   - 包含 DockControl、菜单栏、工具栏、状态栏
   - 参考 UE 编辑器布局

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/MainEditorWindow.axaml.cs`
   - 窗口代码绑定
   - Dock 布局初始化
   - 主题切换逻辑

**依赖关系**：Phase 0

**验证方式**：主窗口显示 Dock 布局，面板可拖拽停靠

---

### Phase 2：核心面板实现 ✅ 已完成

**目标**：实现核心面板的 Avalonia 版本

**设计约束**：
- ViewModel 直接绑定到 View，不经过 Dock 层
- 先实现 SceneBrowser、ContentBrowser、Console 三个面板
- Inspector 在 Phase 4 单独做

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/SceneBrowserView.axaml`
   - 场景浏览器面板
   - 实体树（TreeView + 虚拟化）
   - 搜索栏、工具栏
   - 右键菜单
   - 绑定到 SceneBrowserViewModel

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/SceneBrowserView.axaml.cs`
   - 代码绑定
   - 事件处理（选中、展开、拖拽）
   - 绑定到 SceneBrowserController

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ContentBrowserView.axaml`
   - 内容浏览器面板
   - 左侧目录树（TreeView）
   - 右侧网格/列表视图
   - 面包屑导航
   - 绑定到 ContentBrowserViewModel

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ContentBrowserView.axaml.cs`
   - 代码绑定
   - 目录导航、文件操作
   - 绑定到 ContentBrowserController

5. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ConsoleView.axaml`
   - 控制台面板
   - 日志列表（ListBox + 虚拟化）
   - 工具栏（清空、过滤、自动滚动）
   - 绑定到 ConsolePanelViewModel

6. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ConsoleView.axaml.cs`
   - 代码绑定
   - 日志级别过滤、文本搜索

**依赖关系**：Phase 1

**验证方式**：三个面板能正常显示和交互，数据绑定正确

---

### Phase 3：Viewport 集成 ✅ 已完成

**目标**：实现 3D 视口的 Avalonia 集成

**设计约束**：
- NativeWindowHandle 不进 ViewModel
- 通过 ViewportHostService 管理原生窗口句柄
- IViewportSurface 抽象在 AvaloniaFrontend 实现

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ViewportView.axaml`
   - 视口面板
   - NativeControlHost 嵌入原生窗口
   - 工具栏（聚焦、网格、Gizmo）
   - 绑定到 EditorViewportViewModel

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ViewportView.axaml.cs`
   - 代码绑定
   - NativeControlHost 管理
   - 绑定到 EditorViewportController

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/ViewportHostService.cs`
   - 管理原生窗口句柄
   - 创建 SDL/Win32 子窗口
   - 传递句柄给 Diligent 渲染

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Viewport/IViewportSurface.cs`
   - 视口表面抽象接口
   - 平台无关

5. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Viewport/NativeControlHostSurface.cs`
   - IViewportSurface 的 NativeControlHost 实现
   - 管理 HWND 生命周期

**依赖关系**：Phase 2

**验证方式**：Viewport 能正常显示 3D 场景，交互正常

---

### Phase 3.5：Viewport Overlay（Guizmo）

**目标**：实现 Viewport 上的 Gizmo 操作层

**设计约束**：
- Guizmo 属于 Renderer，不属于 UI
- 放在 `Neverness.Editor.Scene` 或独立模块，不放 AvaloniaFrontend
- Avalonia 只负责把鼠标事件传给 ViewportHostService
- Diligent Renderer 直接绘制 Guizmo Layer
- 未来 ImGui/Avalonia/Web 都能复用同一套 Guizmo

**架构**：
```
Avalonia ViewportView
    ↓ 鼠标事件
ViewportHostService → GuizmoInputBridge（Scene 模块）
    ↓
Diligent Renderer → GuizmoLayer
```

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/ViewportOverlay/ViewportOverlayService.cs`
   - 管理 Guizmo 状态（选中实体、操作模式、坐标系）
   - 接收鼠标输入，转换为 Guizmo 操作
   - 调用 Diligent Renderer 绘制 Guizmo

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/ViewportOverlay/GuizmoInputBridge.cs`
   - 接收来自任意 Frontend 的鼠标输入
   - 处理 Viewport 坐标转换

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Public/IGuizmoInputReceiver.cs`
   - Guizmo 输入接收接口
   - Avalonia/ImGui 都实现此接口

**依赖关系**：Phase 3

**验证方式**：Viewport 中能显示 Guizmo，Translate/Rotate/Scale 操作正常

---

### Phase 4：Inspector 系统 ✅ 已完成

**目标**：实现组件检查器

**设计约束**：
- **不做 PropertyGrid**（当前组件太少，抽象成本太高）
- 直接硬编码每个 Inspector 的 AXAML
- 以后组件多了再考虑 PropertyGrid

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/AvaloniaInspectorBase.cs`
   - Avalonia 版本的 Inspector 基类
   - 替代 ImGui 的 ComponentTypeInspector<T>

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/TransformInspector.axaml`
   - Transform 组件检查器
   - Position/Rotation/Scale 编辑器

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/TransformInspector.axaml.cs`
   - 代码绑定

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/CameraInspector.axaml`
   - Camera 组件检查器

5. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/CameraInspector.axaml.cs`
   - 代码绑定

6. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/SpriteRendererInspector.axaml`
   - SpriteRenderer 组件检查器

7. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/SpriteRendererInspector.axaml.cs`
   - 代码绑定

8. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Inspectors/AvaloniaComponentInspectorRegistry.cs`
   - Avalonia 版本的 Inspector 注册表
   - 自动发现和注册

**依赖关系**：Phase 2（Inspector 面板本身在 Phase 2 已创建，这里补充具体 Inspector 实现）

**验证方式**：Inspector 能正常显示组件属性，编辑后能正确保存

---

### Phase 5：拖放系统 ✅ 已完成（简化版）

**目标**：实现资产拖放系统

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/DragDrop/AvaloniaAssetDragDropService.cs`
   - 实现 IAssetDragDropService
   - Avalonia 拖放 API 集成

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/DragDrop/AvaloniaDropHandler.cs`
   - 拖放处理器
   - 支持文件拖入、资产拖拽

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Converters/AssetDragDataConverter.cs`
   - 资产拖拽数据转换
   - GUID + TypeId 序列化

**依赖关系**：Phase 2

**验证方式**：拖放功能正常工作，资产能正确拖入 Inspector

---

### Phase 6：菜单和工具栏 ✅ 已完成

**目标**：实现完整的菜单和工具栏系统

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/MenuBarView.axaml`
   - 主菜单栏
   - 文件、编辑、视图、工具、帮助菜单
   - 绑定到 EditorMenuRegistry

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/MenuBarView.axaml.cs`
   - 菜单项生成
   - 快捷键绑定
   - 动态菜单

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ToolbarView.axaml`
   - 工具栏
   - 常用操作按钮
   - 绑定到 ToolbarManager

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ToolbarView.axaml.cs`
   - 工具栏按钮生成
   - 状态同步

5. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/StatusBarView.axaml`
   - 状态栏
   - 显示当前状态、进度、警告

**依赖关系**：Phase 2

**验证方式**：菜单和工具栏能正常工作，快捷键响应正确

---

### Phase 7：主题系统 ✅ 已完成

**目标**：实现主题切换系统

**设计约束**：
- **只做 Dark/Light 两个主题**
- 先不支持 Blue/Classic（UI 稳定后再扩展）
- 每个控件只维护 2 套样式

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Themes/Colors.axaml`
   - 颜色资源键定义
   - 参考 Launcher 的 Colors.axaml 结构

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Themes/DarkTheme.axaml`
   - 深色主题颜色定义

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Themes/LightTheme.axaml`
   - 浅色主题颜色定义

4. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/ThemeService.cs`
   - 主题切换服务
   - 运行时切换主题
   - 主题持久化

**依赖关系**：Phase 2（UI 稳定后）

**验证方式**：主题切换 UI 能正常工作，颜色变化正确

---

### Phase 8：本地化系统 ✅ 已完成

**目标**：集成现有的 EditorText 本地化系统，支持运行时语言切换

**设计约束**：
- UI 稳定后再做本地化（避免反复改 Key）
- 复用现有的 EditorText 机制

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Services/LocalizationService.cs`
   - 包装 EditorText，提供 Avalonia 绑定接口
   - 支持运行时语言切换
   - 通知 UI 更新

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Converters/LocalizationConverter.cs`
   - 将 EditorText.Get(key) 转换为 Avalonia 绑定
   - 支持动态更新

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Controls/LocalizedTextBlock.cs`
   - 自定义控件，自动根据当前语言显示文本
   - 绑定到 LocalizationService

4. **新建** `Engine/Resource/Editor/localization/EN_US.txt`
   - 英文本地化文件（参考 ZH_CN.txt 的 key）

**依赖关系**：Phase 7（UI 稳定后）

**验证方式**：语言切换 UI 能正常工作，文本显示正确

---

### Phase 9：上下文菜单 ✅ 已完成

**设计约束**：
- 菜单内容（MenuItem 定义、命令绑定）属于业务逻辑，放在 Core
- 菜单渲染（ContextMenu / MenuItem AXAML）属于 UI，放在 AvaloniaFrontend
- ImGui/Avalonia 共享同一套菜单定义，各自渲染

**架构**：
```
Core（菜单定义）：
  ContentBrowserContextMenuContributor → EditorMenuRegistry
  SceneBrowserContextMenuContributor → EditorMenuRegistry

Avalonia（菜单渲染）：
  AvaloniaContextMenuRenderer → 读取 EditorMenuRegistry → 渲染 ContextMenu
```

**文件变更**：

1. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Core/Private/ContextMenus/ContentBrowserContextMenuContributor.cs`
   - 内容浏览器右键菜单定义
   - 绑定到 ContentBrowserController
   - 注册 MenuItem 到 EditorMenuRegistry

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Core/Private/ContextMenus/SceneBrowserContextMenuContributor.cs`
   - 场景浏览器右键菜单定义
   - 绑定到 SceneBrowserController

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/ContextMenus/AvaloniaContextMenuRenderer.cs`
   - 实现上下文菜单渲染
   - 从 EditorMenuRegistry 读取菜单定义
   - 渲染为 Avalonia ContextMenu

**依赖关系**：Phase 2

**验证方式**：右键菜单能正常显示和操作

---

### Phase 10：EditorApplicationRunner 集成 ✅ 已完成

**目标**：修改编辑器启动流程，支持 AvaloniaFrontend

**设计约束**：
- 启动参数切换 Frontend（`--frontend=avalonia` 或 `--frontend=imgui`）
- ImGui 模式保持现有行为（Developer Tools）

**文件变更**：

1. **修改** `Engine/Source/Managed/Editor/NevernessEditor/EditorApplicationRunner.cs`
   - 添加启动参数解析（`--frontend=avalonia` 或 `--frontend=imgui`）
   - 根据参数选择 Frontend 模块
   - Avalonia 模式下启动 Avalonia 应用
   - ImGui 模式保持现有行为（Developer Tools）

2. **修改** `Engine/Source/Managed/Editor/NevernessEditor/NevernessEditor.csproj`
   - 添加 AvaloniaFrontend 项目引用

3. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Public/AvaloniaEditorHost.cs`
   - Avalonia 编辑器宿主
   - 管理 Avalonia 应用生命周期
   - 与 Native 事件循环集成

**依赖关系**：Phase 0-9

**验证方式**：通过启动参数能切换到 Avalonia 编辑器，功能完整

---

### Phase 11：ImGuiFrontend 降级

**目标**：ImGuiFrontend 不再作为主编辑器，降级为 Developer Tools Frontend

**设计约束**：
- **不删除 ImGuiFrontend**
- 保留所有 ImGui 代码
- 作为 GPU Debug、Texture Viewer、Render Graph 等开发工具的前端
- 不继续扩展业务功能

**文件变更**：

1. **修改** `Engine/Source/Managed/Editor/Neverness.Editor.ImGuiFrontend/README.md`
   - 说明 ImGuiFrontend 的新定位：Developer Tools Frontend
   - 列出保留原因：GPU Debug、Texture Viewer、Render Graph、Experimental Tools

2. **不删除任何文件**

**依赖关系**：Phase 11（AvaloniaFrontend 完全稳定）

**验证方式**：ImGuiFrontend 仍可作为 Developer Tools 启动

**验证方式**：ImGuiFrontend 仍可作为 Developer Tools 启动

---

## 关键文件清单

### Framework 层（FrontendServices 接口）
- `Neverness.Editor.Framework/Public/Services/IDockService.cs`
- `Neverness.Editor.Framework/Public/Services/IWindowService.cs`
- `Neverness.Editor.Framework/Public/Services/INotificationService.cs`

### Core 层（菜单定义）
- `Neverness.Editor.Core/Private/ContextMenus/ContentBrowserContextMenuContributor.cs`
- `Neverness.Editor.Core/Private/ContextMenus/SceneBrowserContextMenuContributor.cs`

### Scene 模块（Guizmo/Viewport Overlay）
- `Neverness.Editor.Scene/Private/ViewportOverlay/ViewportOverlayService.cs`
- `Neverness.Editor.Scene/Private/ViewportOverlay/GuizmoInputBridge.cs`
- `Neverness.Editor.Scene/Public/IGuizmoInputReceiver.cs`

### AvaloniaFrontend 模块（~30 个文件）
- `Neverness.Editor.AvaloniaFrontend/Neverness.Editor.AvaloniaFrontend.csproj`
- `Neverness.Editor.AvaloniaFrontend/Public/AvaloniaFrontendModule.cs`
- `Neverness.Editor.AvaloniaFrontend/Public/AvaloniaViewFactory.cs`
- `Neverness.Editor.AvaloniaFrontend/Public/AvaloniaEditorHost.cs`
- `Neverness.Editor.AvaloniaFrontend/App.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Program.cs`
- `Neverness.Editor.AvaloniaFrontend/Dock/EditorDockFactory.cs`
- `Neverness.Editor.AvaloniaFrontend/Dock/EditorDockLayout.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/MainEditorWindow.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/SceneBrowserView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/ContentBrowserView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/ConsoleView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/ViewportView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/MenuBarView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/ToolbarView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/StatusBarView.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Themes/Colors.axaml`
- `Neverness.Editor.AvaloniaFrontend/Themes/DarkTheme.axaml`
- `Neverness.Editor.AvaloniaFrontend/Themes/LightTheme.axaml`
- `Neverness.Editor.AvaloniaFrontend/Services/ThemeService.cs`
- `Neverness.Editor.AvaloniaFrontend/Services/ViewportHostService.cs`
- `Neverness.Editor.AvaloniaFrontend/Services/AvaloniaDockService.cs`
- `Neverness.Editor.AvaloniaFrontend/Services/AvaloniaWindowService.cs`
- `Neverness.Editor.AvaloniaFrontend/Services/AvaloniaNotificationService.cs`
- `Neverness.Editor.AvaloniaFrontend/Services/LocalizationService.cs`
- `Neverness.Editor.AvaloniaFrontend/Viewport/IViewportSurface.cs`
- `Neverness.Editor.AvaloniaFrontend/Viewport/NativeControlHostSurface.cs`
- `Neverness.Editor.AvaloniaFrontend/Converters/LocalizationConverter.cs`
- `Neverness.Editor.AvaloniaFrontend/Converters/AssetDragDataConverter.cs`
- `Neverness.Editor.AvaloniaFrontend/Controls/LocalizedTextBlock.cs`
- `Neverness.Editor.AvaloniaFrontend/Inspectors/AvaloniaInspectorBase.cs`
- `Neverness.Editor.AvaloniaFrontend/Inspectors/AvaloniaComponentInspectorRegistry.cs`
- `Neverness.Editor.AvaloniaFrontend/Inspectors/TransformInspector.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Inspectors/CameraInspector.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/Inspectors/SpriteRendererInspector.axaml` + `.cs`
- `Neverness.Editor.AvaloniaFrontend/DragDrop/AvaloniaAssetDragDropService.cs`
- `Neverness.Editor.AvaloniaFrontend/DragDrop/AvaloniaDropHandler.cs`
- `Neverness.Editor.AvaloniaFrontend/ContextMenus/AvaloniaContextMenuRenderer.cs`

### 资源文件
- `Resource/Editor/localization/EN_US.txt`

### 需要修改的文件（~2 个）
- `NevernessEditor/NevernessEditor.csproj`（添加 AvaloniaFrontend 引用）
- `NevernessEditor/EditorApplicationRunner.cs`（添加 Frontend 切换逻辑）

### 不删除的文件
- `Neverness.Editor.ImGuiFrontend/` — 保留为 Developer Tools Frontend
- `ThirdParty/Neverness.Editor.ImGui/` — 保留
- `ThirdParty/Neverness.Editor.ImGuiNodeEditor/` — 保留
- `ThirdParty/Neverness.Editor.ImGuizmo/` — 保留
- `Neverness.Editor.ImGuiEx/` — 保留

---

## 执行顺序

```
Phase 0 (脚手架 + FrontendServices 接口)
    │
    v
Phase 1 (Dock 布局)
    │
    v
Phase 2 (核心面板：SceneBrowser, ContentBrowser, Console)
    │
    ├──→ Phase 3 (Viewport + NativeControlHost)
    │        │
    │        v
    │    Phase 3.5 (Guizmo — 在 Scene 模块，不在 Avalonia)
    │
    ├──→ Phase 4 (Inspector：硬编码各组件)
    │
    ├──→ Phase 5 (拖放)
    │
    ├──→ Phase 6 (菜单工具栏)
    │
    └──→ Phase 9 (上下文菜单：Core 定义 + Avalonia 渲染)
              │
              v
         Phase 7 (主题：Dark/Light)
              │
              v
         Phase 8 (本地化)
              │
              v
         Phase 10 (启动集成)
              │
              v
         Phase 11 (ImGuiFrontend 降级为 Developer Tools)
```

**并行策略**：Phase 3/4/5/6/9 可以并行开发，互不依赖。

---

## 与原版的关键差异

| 项目 | 原版 | 最终版 | 原因 |
|------|------|--------|------|
| ImGuiFrontend | 删除 | 保留为 Developer Tools | GPU Debug/Texture Viewer 等工具需要 |
| Dock 设计 | Dock PanelViewModel 桥接 | Dock 只做容器 | 避免耦合 |
| NativeWindowHandle | 进 ViewModel | 通过 ViewportHostService | ViewModel 保持平台无关 |
| PropertyGrid | Phase 4 做 | 延后，先硬编码 | 当前组件太少，抽象成本高 |
| Viewer 系统 | Phase 10 做 | 延后 | 当前阶段不需要 |
| Theme | Dark/Light/Blue/Classic | 先 Dark/Light | 减少维护成本 |
| Localization | Phase 3 | Phase 8（UI 稳定后） | 避免反复改 Key |
| FrontendServices | Phase 8（太晚） | Phase 0（基础设施最先落地） | 避免写完再抽接口导致返工 |
| Guizmo | 无 → 放 AvaloniaFrontend | 放 Scene 模块 | Guizmo 属于 Renderer，不属于 UI，跨 Frontend 复用 |
| ContextMenu | Contributor 放 Avalonia | 定义放 Core，渲染放 Avalonia | 菜单内容是业务逻辑，跨 Frontend 复用 |
| Phase 顺序 | Theme→Localization→Panels | Panels→Viewport→Theme→Localization | 先让功能跑起来 |

---

## 验证方案

### 每个 Phase 的验证
1. **编译验证**：模块能编译通过，无错误
2. **运行验证**：编辑器能启动，对应功能正常
3. **集成验证**：与现有模块（Core, Framework, Scene, Assets）交互正常

### 最终验证
1. **功能完整性**：所有 ImGuiFrontend 的核心功能在 AvaloniaFrontend 中都有对应实现
2. **性能验证**：UI 响应流畅，无卡顿
3. **主题验证**：Dark/Light 主题切换正常
4. **布局验证**：Dock 布局保存/恢复正常
5. **Guizmo 验证**：Viewport 中能操作实体 Transform

---

## 风险和缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| Dock 库 API 不稳定 | 高 | 锁定版本，编写适配层隔离变化 |
| NativeControlHost 跨平台问题 | 中 | 先实现 Windows，后续扩展 |
| Avalonia 性能问题 | 中 | 使用虚拟化、懒加载、数据绑定优化 |
| 与 Native 事件循环冲突 | 高 | 使用 Avalonia 的 DispatcherTimer 或自定义事件泵 |
| Guizmo 输入桥接复杂 | 中 | 先实现 Translate，再扩展 Rotate/Scale |

---

## 参考资源

- **Dock 库文档**：https://github.com/wieslawsoltes/Dock
- **Avalonia 文档**：https://docs.avaloniaui.net/
- **Launcher 实现**：`Engine/Source/Managed/Launcher/NevernessLauncher/`（Avalonia 参考）
- **ImGuiFrontend 实现**：`Engine/Source/Managed/Editor/Neverness.Editor.ImGuiFrontend/`（功能参考）
- **MVVM 架构文档**：`Engine/Docs/Plans/11.ImGuiFrontend_MVVM_Isolation_Plan.md`

---

## 实施进度跟踪

| Phase | 状态 | 完成日期 | 说明 |
|-------|------|----------|------|
| Phase 0：脚手架 + FrontendServices | ✅ 已完成 | 2026-06-12 | csproj、App.axaml、Program.cs、AvaloniaFrontendModule、AvaloniaViewFactory、IDockService/IWindowService/INotificationService + Avalonia 实现 |
| Phase 1：Dock 布局 | ✅ 已完成 | 2026-06-12 | EditorDockFactory（五区域布局）、EditorDockLayout、MainEditorWindow 集成 DockControl + DockFluentTheme |
| Phase 2：核心面板 | ✅ 已完成 | 2026-06-12 | SceneBrowserView（TreeView + 搜索 + 展开/折叠）、ContentBrowserView（目录树 + 文件网格 + 面包屑）、ConsoleView（ListBox + 过滤 + 级别着色） |
| Phase 3：Viewport | ✅ 已完成 | 2026-06-12 | IViewportSurface 接口、NativeControlHostSurface 实现、ViewportHostService、ViewportAvaloniaView 集成 NativeControlHost |
| Phase 3.5：Guizmo | ⬜ 未开始 | - | 放在 Scene 模块，不在 AvaloniaFrontend |
| Phase 4：Inspector | ✅ 已完成 | 2026-06-12 | AvaloniaInspectorBase、TransformInspector、CameraInspector、SpriteRendererInspector、AvaloniaComponentInspectorRegistry |
| Phase 5：拖放 | ✅ 已完成（简化版） | 2026-06-12 | AvaloniaAssetDragDropService、AvaloniaDropHandler。TODO: 适配 Avalonia 12.0 DragDrop API |
| Phase 6：菜单工具栏 | ✅ 已完成 | 2026-06-12 | MenuBarAvaloniaView、ToolbarAvaloniaView、StatusBarAvaloniaView、MainEditorWindow 集成 |
| Phase 7：主题 | ✅ 已完成 | 2026-06-12 | DarkTheme.axaml、LightTheme.axaml、ThemeService（Dark/Light 切换） |
| Phase 8：本地化 | ✅ 已完成 | 2026-06-12 | LocalizationService、LocalizationConverter、LocalizedTextBlock 控件 |
| Phase 9：上下文菜单 | ✅ 已完成 | 2026-06-12 | AvaloniaContextMenuRenderer 实现 IContextMenuRenderer 接口 |
| Phase 10：启动集成 | ✅ 已完成 | 2026-06-12 | EditorApplicationRunner 添加 --frontend 参数支持、AvaloniaEditorHost 生命周期管理、NevernessEditor.csproj 添加 AvaloniaFrontend 引用 |
| Phase 11：ImGuiFrontend 降级 | ⬜ 未开始 | - | |

### 已知问题/待解决

1. **Dock 序列化**：EditorDockLayout 的 SaveLayout/LoadLayout 为 TODO
2. **DragDrop API**：Avalonia 12.0 的 DragDrop API 有变化，当前为简化实现
3. **Avalonia 线程同步**：Avalonia 在独立线程启动，与 Native 事件循环的同步使用 ManualResetEventSlim

---

## Dock 12.0 集成踩坑记录（2026-06-12）

### 问题 1：DockControl 全黑，不渲染任何内容

**现象**：DockControl 放在 XAML 中，Layout 正确设置，但窗口中间全黑，看不到任何面板。

**根因**：Dock 12.0 需要 `DockFluentTheme` 才能渲染控件。没有这个主题，DockControl 的 ControlTemplate 不存在，所有 Dock 元素都不会显示。

**修复**：
```csharp
// App.axaml.cs - OnFrameworkInitializationCompleted 中
Styles.Add(new DockFluentTheme());
```

**参考**：Dock 官方示例 `DockCodeOnlyMvvmSample/Program.cs`

---

### 问题 2：Dock 版本兼容性

**现象**：Dock 11.2.0 → 11.3.2 → 12.0.0.2，API 变化较大。

**关键变化**：
- `IDockable` 接口在 `Dock.Model.Core` 命名空间
- `IRootDock` 在 `Dock.Model.Controls` 命名空间
- `Document`/`Tool` 没有 `Content` 属性，通过 `Context` 存储内容
- `CreateList<T>()` 方法签名：`CreateList<T>(params T[] items)`
- 必须使用 `DockFluentTheme`，不能用 AXAML StyleInclude

**正确版本组合**（2026-06-12）：
```xml
<PackageReference Include="Avalonia" Version="12.0.4" />
<PackageReference Include="Dock.Avalonia" Version="12.0.0.2" />
<PackageReference Include="Dock.Model.Mvvm" Version="12.0.0.2" />
<PackageReference Include="Dock.Avalonia.Themes.Fluent" Version="12.0.0.2" />
```

---

### 问题 3：Dock 布局创建方式

**正确方式**（参考 Dock 官方示例）：
```csharp
var factory = new Factory();

// 创建面板
var doc = new Document { Id = "Id", Title = "Title" };

// 创建 Dock
var docDock = new DocumentDock
{
    Id = "Documents",
    ActiveDockable = doc,
    VisibleDockables = factory.CreateList<IDockable>(doc)
};

// 创建 Root
var root = new RootDock
{
    Id = "Root",
    ActiveDockable = docDock,
    VisibleDockables = factory.CreateList<IDockable>(docDock)
};

// 初始化
factory.InitLayout(root);

// 设置到 DockControl
dockControl.Factory = factory;
dockControl.Layout = root;
```

**错误方式**：
- 不设置 `DockControl.Factory`（必须设置）
- 不调用 `factory.InitLayout(root)`（必须调用）
- 不添加 `DockFluentTheme`（必须添加）

---

### 问题 4：Document 内容渲染

**Dock 12.0 的 Document 没有 Content 属性**。内容通过 DataTemplate 渲染：

```xml
<dock:DockControl x:Name="DockControl" AutoCreateDataTemplates="True">
    <dock:DockControl.DataTemplates>
        <DataTemplate DataType="dockModels:Document">
            <ContentControl Content="{Binding Context}"/>
        </DataTemplate>
    </dock:DockControl.DataTemplates>
</dock:DockControl>
```

代码中设置内容：
```csharp
document.Context = actualControl;  // DataTemplate 会渲染 Context
```

---

### 问题 5：Avalonia 控件线程安全

**现象**：`System.InvalidOperationException: Call from invalid thread`

**根因**：Avalonia 控件必须在 Avalonia UI 线程创建。EditorCompositionRoot.Build() 在主 UI 线程调用，但 Avalonia 控件需要在 Avalonia 线程创建。

**修复**：
```csharp
public IEditorPanel CreateConsoleView(ConsolePanelViewModel viewModel)
{
    return Dispatcher.UIThread.Invoke(() =>
    {
        var view = new ConsolePanelAvaloniaView();
        view.Bind(viewModel);
        return (IEditorPanel)view;
    });
}
```

---

### 问题 6：Avalonia 控件父节点冲突

**现象**：`The control Button already has a visual parent StackPanel while trying to add it as a child of StackPanel`

**根因**：Avalonia 中控件只能有一个父节点。不能把已属于 A 的子控件移动到 B。

**修复**：直接在目标控件中创建子控件，不要复制引用：
```csharp
// 错误：复制引用
foreach (var child in source.Children.ToList())
    target.Children.Add(child);  // 报错！

// 正确：直接创建
AddToolButton("▶", "Play", () => ExecuteCommand("scene.play"));
```

---

### 问题 7：Avalonia 窗口启动顺序

**现象**：`Unable to locate 'Avalonia.Platform.IWindowingPlatform'`

**根因**：MainEditorWindow 在非 Avalonia UI 线程创建，此时平台服务尚未初始化。

**修复**：延迟窗口创建到 `App.OnFrameworkInitializationCompleted`：
```csharp
public override void OnFrameworkInitializationCompleted()
{
    if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
    {
        desktop.MainWindow = new MainEditorWindow();  // 此时平台已初始化
    }
}
```

使用 `ManualResetEventSlim` 同步等待窗口就绪。
