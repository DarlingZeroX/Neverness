# ContentBrowser Avalonia 实施记录

> 状态：基础功能已完成（2026-06-13）

---

## 架构

```
ContentBrowserAvaloniaView (View)
  → ContentBrowserViewModel (状态 + 变更通知)
  → ContentBrowserController (用户交互)
  → IContentBrowserService (抽象接口)
  → ContentBrowserService (实现，包装 ContentBrowser 单例)
  → ContentBrowser (核心：文件系统扫描、数据填充)
```

---

## 已实现功能

### 目录树（左侧 TreeView）
- 调用 `_controller.GetDirectoryTreeRoot()` 获取根节点
- 递归创建 `TreeViewItem`，显示 📁 + 目录名
- 单击节点 → `_controller.OpenDirectory()`（通过 `TreeView.SelectionChanged` 事件）
- 根节点默认展开

### 缩略图网格（右侧 WrapPanel，90x90）
- 调用 `_controller.GetSubdirectories()` 获取子目录
- 调用 `_controller.GetFiles()` 获取文件
- 每个缩略图：图标区域（90x90）+ 文件名（截断省略）
- 双击目录 → 导航进入
- 双击文件 → `_controller.OpenFile()`
- 悬停高亮 + 手型光标
- `WrapPanel` 自动换行

### 面包屑导航
- 将路径拆分为可点击的段
- 点击任意段 → 跳转到该目录
- 根目录 "Assets" 始终可点击
- `ScrollViewer` 包裹，路径过长可滚动

### 工具栏
- ← 后退按钮（`_controller.GoBack()`）
- ↻ 刷新按钮（`_controller.RefreshDirectory()`）
- 搜索框（`_viewModel.SearchFilter`）

### 数据刷新
- `CurrentDirectory` 变更 → 刷新面包屑 + 文件列表
- `SearchFilter` 变更 → 刷新文件列表

### 文件图标映射
| 扩展名 | 图标 | 扩展名 | 图标 |
|--------|------|--------|------|
| .png/.jpg/.bmp | 🖼️ 图片 | .cs/.cpp/.h | 📝 代码 |
| .fbx/.obj/.gltf | 🧊 模型 | .json/.xml/.yaml | 📋 配置 |
| .wav/.mp3/.ogg | 🎵 音频 | .txt/.md | 📄 文本 |
| .mp4/.avi/.mov | 🎬 视频 | .shader/.hlsl | ☀️ 着色器 |
| .scene | 🗺️ 场景 | .html | 🌐 网页 |
| .lua | 📜 脚本 | 其他 | 📄 默认 |

---

## 关键文件

| 文件 | 职责 |
|------|------|
| `Views/ContentBrowserAvaloniaView.cs` | Avalonia 视图实现 |
| `ViewModels/ContentBrowserViewModel.cs` | ViewModel（CurrentDirectory, SearchFilter 等） |
| `Controllers/ContentBrowserController.cs` | Controller（OpenDirectory, GoBack, GetFiles 等） |
| `Public/IContentBrowserService.cs` | 服务接口 + ContentDirectoryNode/ContentFileNode 数据模型 |
| `ImGuiFrontend/Views/ContentBrowserImGuiView.cs` | ImGui 版本（功能参考） |

---

## 踩坑记录

### 问题 1：Bind() 中使用 _controller 为 null

**现象**：目录树和文件列表不显示任何内容。

**根因**：`Bind()` 在 `SetController()` 之前调用，`_controller` 还是 null。

**修复**：数据加载（`RefreshDirectoryTree`、`RefreshFileList`、`RefreshBreadcrumb`）移到 `SetController()` 中执行。

### 问题 2：Dock 命名空间冲突

**现象**：`Dock.Top`、`Dock.Right` 编译错误。

**根因**：项目引用了 `Dock.Avalonia` 库，`Dock` 被解析为库命名空间而非 `Avalonia.Controls.Dock` 枚举。

**修复**：使用完整限定名 `Avalonia.Controls.Dock.Top`。

### 问题 3：TextBox.Watermark 废弃

**现象**：`CS0618: "TextBox.Watermark" 已过时`。

**修复**：改用 `PlaceholderText`。

### 问题 4：CursorType 不存在

**现象**：`Avalonia.Input.CursorType` 编译错误。

**修复**：Avalonia 12.0 中改为 `Avalonia.Input.StandardCursorType`。

### 问题 5：TreeViewItem 双击不触发导航

**现象**：给 `TreeViewItem` 绑定 `DoubleTapped` 事件，点击目录树节点不触发导航。

**根因**：`TreeViewItem` 的点击事件被展开/折叠逻辑消耗，`DoubleTapped` 无法到达。

**修复**：改用 `TreeView.SelectionChanged` 事件，单击即触发导航：
```csharp
_directoryTree.SelectionChanged += (_, e) =>
{
    if (e.AddedItems[0] is TreeViewItem item && item.Tag is string path)
        _controller.OpenDirectory(path);
};
```
注意：`RefreshDirectoryTree()` 中需先 `-=` 再 `+=` 防止事件重复注册。

---

## 待实现功能

1. **搜索过滤**：`SearchFilter` 变更时实际过滤文件列表（当前只是刷新）
2. **目录树高亮**：`CurrentDirectory` 变更时高亮目录树中对应的节点
3. **右键菜单**：Open、Rename、Delete、Copy Path、New Folder
4. **资产拖拽**：从 ContentBrowser 拖拽资产到 Inspector
5. **网格/列表切换**：支持 Grid 和 List 两种视图模式
6. **缩略图渲染**：用实际资产缩略图替代 emoji 图标
7. **选中状态**：点击缩略图高亮选中，支持多选

---

## 参考资源

- **ImGui 版本**：`Neverness.Editor.ImGuiFrontend/Views/ContentBrowserImGuiView.cs`
- **Dock 文档**：[AvaloniaFrontend_Dock_Guide.md](AvaloniaFrontend_Dock_Guide.md)
