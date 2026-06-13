# AvaloniaFrontend Dock 集成指南

> Dock 库版本：wieslawsoltes/Dock 12.0（Dock.Avalonia 12.0.0.2 + Dock.Model.Mvvm 12.0.0.2）
> Avalonia 版本：12.0.4

---

## 架构设计

### 原则：Dock 只做容器，不参与业务

```
正确：
  Core ViewModel → Avalonia View → DockControl（容器）

错误：
  Core ViewModel → Dock PanelViewModel → DockControl
```

Dock 只是布局容器，不参与业务数据流。

### 当前布局结构

```
RootDock ("Layout")
  └── ProportionalDock ("Root", Vertical)
        ├── ProportionalDock ("Middle", Horizontal)
        │     ├── ToolDock ("Left", Alignment.Left, Proportion=0.2)
        │     │     └── Document: SceneBrowser
        │     ├── ProportionalDockSplitter ("Split1")
        │     ├── DocumentDock ("Center")
        │     │     └── Document: Viewport
        │     ├── ProportionalDockSplitter ("Split2")
        │     └── ToolDock ("Right", Alignment.Right, Proportion=0.25)
        │           └── Document: Inspector
        ├── ProportionalDockSplitter ("Split3")
        └── ToolDock ("Bottom", Alignment.Bottom, Proportion=0.3)
              ├── Document: ContentBrowser
              └── Document: Console
```

### 关键文件

| 文件 | 职责 |
|------|------|
| `Dock/EditorDockFactory.cs` | 工厂类，创建默认布局，配置浮动窗口定位器 |
| `Dock/EditorDockLayout.cs` | 布局序列化/反序列化（TODO） |
| `Views/MainEditorWindow.axaml` | DockControl 声明 |
| `Views/MainEditorWindow.axaml.cs` | Dock 初始化、HostWindowFactory 配置 |
| `App.axaml.cs` | DockFluentTheme 注册 |
| `Services/AvaloniaDockService.cs` | IDockService 实现（TODO） |

---

## 版本组合

```xml
<PackageReference Include="Avalonia" Version="12.0.4" />
<PackageReference Include="Dock.Avalonia" Version="12.0.0.2" />
<PackageReference Include="Dock.Model.Mvvm" Version="12.0.0.2" />
<PackageReference Include="Dock.Avalonia.Themes.Fluent" Version="12.0.0.2" />
```

---

## 踩坑记录

### 问题 1：DockControl 全黑，不渲染任何内容

**现象**：DockControl 放在 XAML 中，Layout 正确设置，但窗口中间全黑，看不到任何面板。

**根因**：Dock 12.0 需要 `DockFluentTheme` 才能渲染控件。没有这个主题，DockControl 的 ControlTemplate 不存在，所有 Dock 元素都不会显示。

**修复**：
```csharp
// App.axaml.cs - OnFrameworkInitializationCompleted 中
Styles.Add(new DockFluentTheme());
```

---

### 问题 2：DataTemplate 缺少 DataType

**现象**：`System.InvalidOperationException: DataTemplate inside of DataTemplates must have a DataType set.`

**根因**：Dock 12.0 要求 `DataTemplates` 集合内的 `DataTemplate` 必须设置 `DataType` 属性。

**修复**：如果已设置 `AutoCreateDataTemplates="True"`，删除手动定义的空 `DataTemplate`：
```xml
<!-- 错误 -->
<dock:DockControl AutoCreateDataTemplates="True">
    <dock:DockControl.DataTemplates>
        <DataTemplate>  <!-- 缺少 DataType -->
            <ContentControl Content="{Binding}"/>
        </DataTemplate>
    </dock:DockControl.DataTemplates>
</dock:DockControl>

<!-- 正确：AutoCreateDataTemplates 会自动处理 -->
<dock:DockControl AutoCreateDataTemplates="True"/>
```

---

### 问题 3：Document 内容渲染

**Dock 12.0 的 Document 没有 Content 属性**。内容通过 `Context` 属性 + DataTemplate 渲染：

```csharp
// 设置内容
document.Context = actualControl;  // DataTemplate 会渲染 Context
```

---

### 问题 4：浮动窗口——面板拖拽后消失

**现象**：拖拽面板标签到空白区域，面板直接消失，不会变成独立窗口。

**根因**：缺少两个关键配置：
1. `DefaultHostWindowLocator` 未设置 — `FloatDockable` 创建 `DockWindow` 模型时需要此定位器获取 `IHostWindow` 实例
2. `DocumentDock.EnableWindowDrag` 未启用 — 拖拽标签触发浮动需要此属性

**修复**（三处配置缺一不可）：

```csharp
// 1. EditorDockFactory 构造函数——注册浮动窗口定位器
public EditorDockFactory()
{
    DefaultHostWindowLocator = () => new HostWindow();
}

// 2. DocumentDock 启用标签拖拽
var centerDock = new DocumentDock
{
    Id = "Center",
    EnableWindowDrag = true,  // 允许拖拽标签浮动
    ...
};

// 3. Document 启用浮动能力
var doc = new Document { Id = "Id", Title = "Title", CanFloat = true };

// 4. RootDock 配置浮动窗口模式和 Windows 集合
var root = new RootDock
{
    Id = "Layout",
    FloatingWindowHostMode = DockFloatingWindowHostMode.Native,  // 原生 OS 窗口
    Windows = CreateList<IDockWindow>(),  // 浮动窗口跟踪集合
    ...
};
```

```csharp
// 5. MainEditorWindow.axaml.cs——配置 HostWindowFactory
DockControl.HostWindowFactory = () => new HostWindow
{
    IsToolWindow = true,                    // 工具窗口样式，不显示在任务栏
    ToolChromeControlsWholeWindow = true,   // Chrome 覆盖整个窗口
};
```

**浮动窗口工作流程**：
```
用户拖拽标签 → FloatDockable(dockable)
  → DefaultHostWindowLocator() 创建 IHostWindow 模型
  → 创建 DockWindow 模型，添加到 RootDock.Windows
  → DockControl 检测到新窗口
  → HostWindowFactory() 创建 Avalonia HostWindow 控件
  → 绑定并显示原生 OS 窗口
```

**浮动窗口模式**：
- `DockFloatingWindowHostMode.Native` — 原生 OS 窗口，独立于主窗口，可拖到其他显示器（推荐，UE 风格）
- `DockFloatingWindowHostMode.Managed` — 应用内窗口，限制在主窗口范围内

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

---

### 问题 8：Dock 版本兼容性

**现象**：Dock 11.2.0 → 11.3.2 → 12.0.0.2，API 变化较大。

**关键变化**：
- `IDockable` 接口在 `Dock.Model.Core` 命名空间
- `IRootDock` 在 `Dock.Model.Controls` 命名空间
- `Document`/`Tool` 没有 `Content` 属性，通过 `Context` 存储内容
- `CreateList<T>()` 方法签名：`CreateList<T>(params T[] items)`
- 必须使用 `DockFluentTheme`，不能用 AXAML StyleInclude

---

## 正确的 Dock 布局创建方式

参考 Dock 官方示例 `DockCodeOnlyMvvmSample`：

```csharp
public class EditorDockFactory : Factory
{
    public EditorDockFactory()
    {
        // 浮动窗口定位器（必须）
        DefaultHostWindowLocator = () => new HostWindow();
    }

    public IRootDock CreateDefaultLayout()
    {
        // 创建面板（启用浮动）
        var doc = new Document { Id = "Id", Title = "Title", CanFloat = true };

        // 创建 DocumentDock（启用标签拖拽）
        var docDock = new DocumentDock
        {
            Id = "Documents",
            ActiveDockable = doc,
            VisibleDockables = CreateList<IDockable>(doc),
            EnableWindowDrag = true
        };

        // 创建 RootDock（配置浮动窗口）
        var root = new RootDock
        {
            Id = "Root",
            ActiveDockable = docDock,
            VisibleDockables = CreateList<IDockable>(docDock),
            FloatingWindowHostMode = DockFloatingWindowHostMode.Native,
            Windows = CreateList<IDockWindow>()
        };

        // 初始化布局（必须）
        InitLayout(root);

        return root;
    }
}
```

```csharp
// MainEditorWindow 构造函数
var factory = new EditorDockFactory();
var layout = factory.CreateDefaultLayout();

DockControl.Factory = factory;
DockControl.Layout = layout;

// 浮动窗口工厂（必须）
DockControl.HostWindowFactory = () => new HostWindow
{
    IsToolWindow = true,
    ToolChromeControlsWholeWindow = true,
};
```

**常见错误**：
- 不设置 `DockControl.Factory`（必须设置）
- 不调用 `factory.InitLayout(root)`（必须调用）
- 不添加 `DockFluentTheme`（必须添加）
- 不设置 `DefaultHostWindowLocator`（浮动窗口不工作）
- 不设置 `EnableWindowDrag`（拖拽标签不触发浮动）

---

### 问题 9：Dock 面板内容不显示（DataTemplate 机制）

**现象**：Dock 布局正常渲染（标签、分割线都可见），但所有面板内容区域为空。

**根因**：Dock 12.0 的 `AutoCreateDataTemplates="True"` 为 `Document` 类型创建的默认模板**不渲染 `Context` 属性**。需要手动注册 `FuncDataTemplate<Document>` 并插入到 `DockControl.DataTemplates` 最前面。

**关键发现**：
1. `AutoCreateDataTemplates="True"` 为布局类型（RootDock、ToolDock 等）创建正确的模板
2. 但它为 `Document` 创建的模板不渲染 `Context`
3. Application 级别的 DataTemplate 优先级低于 DockControl 级别
4. 必须用 `DockControl.DataTemplates.Insert(0, ...)` 插入到最前面

**修复**：
```csharp
// 在 DockControl.Layout 设置之后注册
DockControl.DataTemplates.Insert(0, new FuncDataTemplate(
    typeof(Document),
    (data, _) =>
    {
        if (data is Document doc && doc.Context is Control ctrl)
            return ctrl;
        return new TextBlock { Text = "No Content" };
    }));
```

---

### 问题 10：UserControl 内容不显示（Content 未设置）

**现象**：FuncDataTemplate 正确返回了视图控件，但 Dock 面板仍然为空。甚至返回红色方块（带 MinWidth/MinHeight）能显示，但返回实际视图不行。

**根因**：所有 AvaloniaView 继承 `UserControl`，但 `Bind()` 方法创建了控件树（DockPanel、TreeView 等）却**没有设置 `this.Content = panel`**。`UserControl` 没有 `Content` 就是空的，零尺寸。

**修复**：在每个 View 的 `Bind()` 方法末尾添加 `Content = panel`：
```csharp
public override void Bind(object viewModel)
{
    _viewModel = (SomeViewModel)viewModel;
    var panel = new DockPanel();
    // ... 创建子控件 ...
    panel.Children.Add(someControl);

    Content = panel;  // ← 必须设置！
}
```

**教训**：Avalonia 的 `UserControl` 必须设置 `Content` 才有内容。不像 WPF 的 `UserControl` 有 XAML 模板，Avalonia 的纯代码 UserControl 默认 Content 为 null。

---

### 问题 11：NativeControlHost 需要应用 manifest

**现象**：`System.InvalidOperationException: Unable to create child window for native control host. Application manifest with supported OS list might be required.`

**根因**：Avalonia 的 `NativeControlHost` 在 Windows 上需要应用 manifest 声明支持的 OS 版本。

**修复**：暂时跳过 `NativeControlHost`，用占位面板代替。后续需要添加 `app.manifest` 文件。

---

## Dock 内容渲染完整流程

```
1. EditorDockFactory 创建 Document（Context = null）
2. DockControl.Layout = layout → AutoCreateDataTemplates 为布局类型创建模板
3. DockControl.DataTemplates.Insert(0, FuncDataTemplate<Document>) → 注册 Document 模板
4. Dock 渲染 Document → FuncDataTemplate 被调用 → 返回 Context（此时为 null，显示 "No Content"）
5. EditorCompositionRoot.Build() 创建 View
6. SetPanelContent() 设置 Document.Context = View
7. 由于 FuncDataTemplate 已返回，需要触发重新渲染
   → 如果用绑定方式：ContentControl.Bind(ContentProperty, "Context") 自动更新
   → 如果用直接返回方式：需要确保 Context 在 FuncDataTemplate 调用前已设置
```

**当前实现**：FuncDataTemplate 在 Layout 设置后注册，SetPanelContent 在之后调用。由于 Dock 的 DeferredContentControl 机制，FuncDataTemplate 实际在内容可见时才调用，此时 Context 已设置。

---

## 待实现功能

1. **布局持久化**：`EditorDockLayout.SaveLayout/LoadLayout` 为 TODO，需使用 `Dock.Serializer.Newtonsoft`
2. **AvaloniaDockService**：所有方法为 TODO 空壳
3. **ToolDock 浮动**：当前只有 DocumentDock 支持标签拖拽浮动，ToolDock 需要右键菜单或其他触发方式

---

## 参考资源

- **Dock 库文档**：https://github.com/wieslawsoltes/Dock
- **Dock 官方示例**：`DockCodeOnlyMvvmSample`
- **Avalonia 文档**：https://docs.avaloniaui.net/
