# ContentBrowserThumbnailGrid 重构

**日期**: 2026-06-22
**类型**: UI 组件拆分重构

## 1. 背景

`ContentBrowserThumbnailGrid.cs` 原文件 677 行，包含多种职责：
- Grid UI 布局
- 缩略图创建
- 选择管理
- 拖拽处理
- 右键菜单
- 内联重命名
- 文件信息映射

随着功能扩展（多选框选、拖到目录、虚拟化、资产预览缓存等），需要将职责清晰分离。

## 2. 拆分目标

- **UI 组件 + 行为控制器** 分离
- 缩略图 UI 可被 AssetPicker、Inspector 等复用
- 为后续扩展（虚拟化、多选等）做准备

## 3. 最终结构

```
Views/ContentBrowser/
├─ ContentBrowserThumbnailGrid.cs      ~230 行  容器
└─ ThumbnailGrid/
   ├─ ThumbnailView.cs                ~130 行  纯 UI（可复用）
   ├─ ThumbnailPresenter.cs           ~220 行  交互逻辑
   ├─ ThumbnailRenameService.cs       ~130 行  内联重命名
   └─ ThumbnailFileInfoProvider.cs    ~50 行   文件信息映射
```

## 4. 各文件职责

### 4.1 ContentBrowserThumbnailGrid.cs（容器）

**职责**: 容器布局、文件列表刷新、协调子组件

**保留内容**:
- `Create()` - 搭建 ScrollViewer/WrapPanel/阴影/拖拽高亮
- `Refresh()` - 遍历文件，调用 ThumbnailView.Create()
- `ClearSelection()` - 委托到 Presenter
- `SetSelectedItem()` - 委托到 Presenter
- `BeginRename()` - 委托到 RenameService
- `Dispose()` - 释放资源

**事件转发**:
- `OnContextMenuRequested` → 转发到 Presenter
- `OnBackgroundContextMenuRequested` → 转发到 Presenter
- `OnRenameCommitted` → 转发到 RenameService

### 4.2 ThumbnailView.cs（纯 UI）

**职责**: 创建缩略图的视觉结构，不绑定交互事件

**核心方法**:
```csharp
internal static Control Create(string name, string icon, string path, 
    bool isDirectory, string typeLabel, bool isSelected, IBrush? badgeColor = null)
```

**可复用性**: 可被 ContentBrowser、AssetPicker、Inspector 等复用

**视觉更新**:
- `UpdateSelectionVisual()` - 更新选中状态
- `UpdateHoverVisual()` - 更新悬停状态

### 4.3 ThumbnailPresenter.cs（交互）

**职责**: 管理缩略图的点击、拖拽、悬停、右键等交互行为

**状态管理**:
- `_selectedPaths` - 选中路径集合
- `_thumbnailBorders` - 缩略图 Border 映射
- `_thumbnailIsDirectory` - 目录标记映射
- `_isDragPending` / `_dragStartPos` / `_dragPressedArgs` - 拖拽状态
- `_selectedItemPath` / `_selectedItemName` / `_selectedItemIsDirectory` - 右键菜单状态

**核心方法**:
- `AttachEvents()` - 为缩略图绑定交互事件
- `ClearSelection()` - 清除选中状态
- `SetSelectedItem()` - 设置右键菜单选中项
- `OnRubberBandSelectionChanged()` - 框选变化回调
- `OnFileAreaPointerReleased()` - 空白区域右键处理

**事件**:
- `OnContextMenuRequested` - 缩略图右键菜单
- `OnBackgroundContextMenuRequested` - 背景右键菜单

### 4.4 ThumbnailRenameService.cs（重命名）

**职责**: 处理文件名的内联编辑

**状态**:
- `_activeTextBox` - 当前编辑框
- `_activeNameLabel` - 当前文件名 TextBlock
- `_activePath` / `_activeCurrentName` - 当前重命名目标

**核心方法**:
- `BeginRename()` - 开始内联重命名
- `CancelRename()` - 取消重命名
- `IsRenaming` - 是否正在重命名

**事件**:
- `OnRenameCommitted` - 重命名提交事件

### 4.5 ThumbnailFileInfoProvider.cs（文件信息）

**职责**: 根据扩展名/资产类型返回图标、标签和徽章颜色

**核心方法**:
```csharp
internal static (string Icon, string Label, IBrush BadgeColor) GetInfo(
    string extension, string? assetType)
```

**扩展性**: 新增资产类型只需在此文件添加映射

## 5. 接口兼容性

原 `ContentBrowserThumbnailGrid` 的公共接口保持不变：
- `SelectedPaths` - 通过委托到 Presenter
- `SelectedItemPath` / `SelectedItemName` / `SelectedItemIsDirectory` - 通过委托到 Presenter
- `OnContextMenuRequested` / `OnBackgroundContextMenuRequested` - 事件转发
- `OnRenameCommitted` - 事件转发

## 6. 编译验证

- ✅ 0 错误
- ✅ 接口兼容
- ✅ 功能不变

## 7. 后续扩展点

1. **虚拟化**: WrapPanel → ItemsRepeater/VirtualizingPanel，ThumbnailView 可直接复用
2. **多选增强**: Shift 选择范围、Ctrl 切换选择，在 Presenter 中扩展
3. **拖拽增强**: 拖到目录、拖到子目录，在 Presenter 中扩展
4. **资产预览缓存**: 在 ThumbnailView 中扩展图片加载
5. **AssetPicker/Inspector 复用**: 直接使用 ThumbnailView.Create()
