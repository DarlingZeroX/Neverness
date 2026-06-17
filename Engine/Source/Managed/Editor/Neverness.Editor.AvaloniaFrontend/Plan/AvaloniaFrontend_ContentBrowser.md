# ContentBrowser Avalonia 实施记录

> 状态：Unreal 风格视觉重写 + 框选 + 搜索 + 选中 + 目录树高亮 + 资产拖拽（2026-06-16）

---

## 架构

```
ContentBrowserAvaloniaView (View)
  → ContentBrowserViewModel (状态 + 变更通知)
  → ContentBrowserController (用户交互)
  → IContentBrowserService (抽象接口)
  → ContentBrowserService (实现，包装 ContentBrowser 单例)
  → ContentBrowser (核心：文件系统扫描、数据填充)
  → RubberBandSelection (鼠标框选逻辑)
```

---

## 布局（Unreal Content Browser 风格）

```
┌──────────────────────────────────────────────────┐
│ [◀][▶]  Content/Scenes            [🔍 Search]    │ ← 工具栏 (BgToolbar #3B3B3B)
├─────────────┬─┬──────────────────────────────────┤
│ Content     │ │ ┌──────┐ ┌──────┐ ┌──────┐      │
│  ▾ Scenes   │ │ │  🗺️  │ │  🧊  │ │  📋  │      │
│  ▾ Prefabs  │▓│ │SCENE │ │PREFB │ │MATER │      │
│             │▓│ └──────┘ └──────┘ └──────┘      │
│             │▓│ ┌──────┐ ┌──────┐               │
│             │▓│ │  🖼️  │ │  🎵  │               │
│             │ │ │ TEX  │ │ AUD  │               │
│             │ │ └──────┘ └──────┘               │
├─────────────┴─┴──────────────────────────────────┤
│ Content/Scenes                     2 items sel.  │ ← 状态栏 (BgStatusBar #252525)
└──────────────────────────────────────────────────┘
  ▓ = 面板内阴影 (BoxShadow Inset)
```

---

## 已实现功能

### 视觉风格（2026-06-16 Unreal 风格重写）

**配色方案**：
| 区域 | 颜色 |
|------|------|
| 主背景 | `#2B2B2B` |
| 面板背景 | `#353535` |
| 工具栏 | `#3B3B3B` |
| 状态栏 | `#252525` |
| 输入框 | `#1E1E1E` |
| 缩略图（文件） | `#404040` |
| 缩略图（目录） | `#484848` |
| 缩略图（悬停） | `#505050` |
| 缩略图（选中） | `#2A5D9E` |
| 选中边框 | `#2196F3` |
| 文字主色 | `#CCCCCC` |
| 文字次色 | `#888888` |
| 分割线 | `#1A1A1A` |

**缩略图卡片**（80×118）：
- 整体圆角 Border（4px），一个卡片 = 一个整体
- 图标区 80×80（居中，FontSize 32）
- 文件名（11px，截断省略，白色）
- 资产类型标签（9px，粗体，彩色）：SCENE(绿) / PREFB(蓝) / MATER(橙) / TEX(紫) / AUD(红) / CODE(青)
- BoxShadow 阴影（黑色 60%，模糊 6px，偏移 Y+2）
- 选中：蓝色边框 2px + 蓝色背景 `#2A5D9E`

**面板内阴影**：
- 左面板（目录树）：右侧内阴影 `BoxShadow Inset, Blur=14, OffsetX=-8`，通过 Grid.ClipToBounds 裁剪左侧
- 右面板（缩略图）：左侧内阴影 `BoxShadow Inset, Blur=14, OffsetX=+8`，通过 Grid.ClipToBounds 裁剪右侧

**可拖拽分割线**：左右面板之间 `GridSplitter`（4px），MinWidth=120px

**工具栏**：
- ◀▶ 导航按钮（圆角 3px，hover 高亮 `#4A4A4A`）
- 面包屑内嵌（› 分隔符，hover 变亮）
- 搜索框（圆角 3px，PlaceholderText `🔍 Search...`）

**状态栏**：底部显示相对路径 + "N items selected"

**目录树**：
- 标题 "Content"（粗体，次色）
- 展开/折叠箭头 ▾/▸
- 📁 文件夹图标
- 无背景（透明）

### 目录树（左侧 TreeView）
- 调用 `_controller.GetDirectoryTreeRoot()` 获取根节点
- 递归创建 `TreeViewItem`
- 单击节点 → `_controller.OpenDirectory()`（`TreeView.SelectionChanged`）
- 根节点默认展开
- `CurrentDirectory` 变更时自动高亮对应节点 + 展开父节点

### 缩略图网格（右侧 WrapPanel）
- 调用 `_controller.GetSubdirectories()` 获取子目录
- 调用 `_controller.GetFiles()` 获取文件
- WrapPanel 宽度绑定 ScrollViewer.ViewportWidth → 自动换行 → 垂直滚动
- 双击目录 → 导航进入
- 双击文件 → `_controller.OpenFile()`
- 悬停高亮（`#505050`）+ 手型光标

### 面包屑导航
- 路径拆分为可点击段（› 分隔）
- 点击任意段 → 跳转到该目录
- 根目录 "Content" 始终可点击
- hover 时文字变亮

### 搜索过滤
- `SearchFilter` 变更时按名称过滤（不区分大小写）
- 搜索时也显示匹配的子目录
- 空搜索框显示全部

### 选中状态
- 单击 → 单选（蓝色边框 + 蓝色背景）
- Ctrl+单击 → 多选切换
- 点击空白 → 取消所有选中
- 选中不会被悬停覆盖
- 切换目录自动清除

### 鼠标框选（RubberBandSelection）
- 空白区域拖拽 → 蓝色半透明矩形
- 框内项目实时高亮选中
- Ctrl+拖拽追加选中
- 3px 阈值区分单击/拖拽
- 边缘 30px 自动滚动（12px/次，30ms 间隔）
- 指针捕获确保鼠标移出也能收到事件
- 详见 ContentBrowser_RubberBand_Selection.md

### 资产拖拽架构
```
ContentBrowserThumbnailGrid          InspectorAvaloniaView
  缩略图 PointerPressed              ┌─ CreateAssetDropTarget()
    → 记录 _dragStartPos             │   DragDrop.AddDragEnterHandler
  缩略图 PointerMoved                │   DragDrop.AddDragOverHandler
    → 阈值 > 5px                     │   DragDrop.AddDragLeaveHandler
    → new DataObject                 │   DragDrop.AddDropHandler
    → DoDragDrop(Copy)               │     → onAssetDropped(path)
    → 携带 SystemPath + VirtualPath  │     → 更新显示文字
                                     └─ DragDrop.SetAllowDrop = true

数据格式：
  "nnasset/systempath"  → 文件系统绝对路径
  "nnasset/virtualpath" → 资产虚拟路径（优先使用）

AssetDragFormats 辅助类：
  GetAssetPath(DataObject)       → 优先虚拟路径，回退系统路径
  GetAssetPathFromDrag(DragEventArgs) → 从拖拽事件提取路径
```

### 右键菜单
- 缩略图右键 → Item 菜单（Rename / Remove / Show in Explorer / Copy Name）
- 空白区域右键 → Background 菜单（Create Directory / Refresh / Show in Explorer）
- 目录树节点右键 → Background 菜单

### 资产拖拽（ContentBrowser → Inspector）
- 缩略图左键按住 + 移动超过 5px 阈值 → 启动 `DragDrop.DoDragDrop`
- 携带两种数据格式：`nnasset/systempath`（绝对路径）+ `nnasset/virtualpath`（虚拟路径）
- Inspector 侧通过 `CreateAssetDropTarget()` 创建可接收拖拽的资产引用字段
- 拖拽进入时蓝色高亮边框（`#2196F3`），离开时恢复
- 放下后显示资产名称（不含扩展名），触发 `onAssetDropped` 回调
- 不与 RubberBandSelection 冲突：框选仅在空白区域触发，拖拽仅在缩略图上触发
- 双击时自动取消拖拽状态，防止误触发
- 当前已在 SpriteRendererInspector 的 Texture 字段接入

### 文件信息映射
| 扩展名 | 图标 | 类型标 | 颜色 |
|--------|------|--------|------|
| .png/.jpg/.tga/.hdr | 🖼️ | TEX | 紫 |
| .fbx/.obj/.gltf | 🧊 | MESH | 灰 |
| .wav/.mp3/.ogg | 🎵 | AUD | 红 |
| .mp4/.avi/.mov | 🎬 | VID | 红 |
| .cs/.cpp/.h/.py | 📝 | CODE | 青 |
| .json/.xml/.yaml | 📋 | CFG | 灰 |
| .shader/.hlsl | ☀️ | SHD | 橙 |
| .scene | 🗺️ | SCENE | 绿 |
| .prefab | 🧊 | PREFB | 蓝 |
| .mat | 📋 | MATER | 橙 |
| 其他 | 📄 | FILE | 灰 |

---

## 关键文件

| 文件 | 职责 |
|------|------|
| `Views/ContentBrowserAvaloniaView.cs` | Avalonia 视图实现（Unreal 风格） |
| `ContentBrowser/RubberBandSelection.cs` | 鼠标框选逻辑 |
| `ContentBrowser/AssetDragFormats.cs` | 资产拖拽数据格式常量 + 辅助方法 |
| `ViewModels/ContentBrowserViewModel.cs` | ViewModel（CurrentDirectory, SearchFilter 等） |
| `Controllers/ContentBrowserController.cs` | Controller（OpenDirectory, GoBack, GetFiles, GetAssetVirtualPath 等） |
| `Public/IContentBrowserService.cs` | 服务接口 + 数据模型 |
| `ImGuiFrontend/Views/ContentBrowserImGuiView.cs` | ImGui 版本（功能参考） |

---

## 踩坑记录

### 问题 1：Bind() 中使用 _controller 为 null
**修复**：数据加载移到 `SetController()` 中执行。

### 问题 2：Dock 命名空间冲突
**修复**：使用完整限定名 `Avalonia.Controls.Dock.Top`。

### 问题 3：TextBox.Watermark 废弃
**修复**：改用 `PlaceholderText`。

### 问题 4：CursorType 不存在
**修复**：Avalonia 12.0 中改为 `StandardCursorType`。

### 问题 5：TreeViewItem 双击不触发导航
**修复**：改用 `TreeView.SelectionChanged` 事件，先 `-=` 再 `+=` 防止重复注册。

### 问题 6：Canvas 不接收鼠标事件
**现象**：RubberBandSelection 的 PointerPressed 不触发。
**根因**：Avalonia 只对有 Background 的控件做命中测试。
**修复**：Canvas 添加 `Background = Brushes.Transparent`。

### 问题 7：WrapPanel 不换行
**现象**：缩略图一直向右扩展，不垂直滚动。
**根因**：Canvas 给子控件无限宽度，WrapPanel 永远不换行。
**修复**：将 Canvas 改为 Grid（Grid 约束子控件宽度），WrapPanel 自动换行。

### 问题 8：BoxShadow 被 ClipToBounds 裁剪
**现象**：缩略图卡片阴影不显示。
**根因**：`ClipToBounds = true` 裁剪了 Border 外部的阴影。
**修复**：外层 Border（提供阴影空间 + Padding）包裹内层 Border（卡片内容）。

### 问题 9：BoxShadows 不支持多阴影叠加
**现象**：`new BoxShadows(shadow1, shadow2, shadow3, shadow4)` 编译错误。
**根因**：Avalonia `BoxShadows` 构造函数只接受单个 `BoxShadow`。
**解决**：单面板单阴影，通过 OffsetX 方向控制。

### 问题 10：内阴影向两侧扩散
**现象**：左面板左侧也有阴影。
**根因**：`BoxShadow` 的 Blur 向四周扩散，无法只向一侧。
**修复**：用两个层——底层 Border 的 BoxShadow 推向目标侧（OffsetX），上层 Grid 的 ClipToBounds 裁剪另一侧。

---

## 待实现功能

1. ~~**搜索过滤**~~ ✅
2. ~~**目录树高亮**~~ ✅
3. **右键菜单**：部分已实现，需完善
4. ~~**资产拖拽**~~ ✅ 基础框架已实现（拖拽源 + Inspector Drop Target），SpriteRenderer 已接入
5. **网格/列表切换**：支持 Grid 和 List 两种视图模式
6. **缩略图渲染**：用实际资产缩略图替代 emoji 图标
7. ~~**选中状态**~~ ✅
8. ~~**鼠标框选**~~ ✅
9. ~~**Unreal 风格视觉**~~ ✅

---

## 变更记录

| 日期 | 版本 | 变更内容 |
|------|------|----------|
| 2026-06-13 | v1.0 | 基础功能：目录树、缩略图网格、面包屑、工具栏、数据刷新 |
| 2026-06-16 | v2.0 | 搜索过滤、选中状态（单击/Ctrl多选）、目录树高亮 |
| 2026-06-16 | v3.0 | 鼠标框选（RubberBandSelection）、Canvas→Grid 修复换行 |
| 2026-06-16 | v4.0 | Unreal 风格视觉重写：配色、缩略图卡片（圆角+阴影+类型标签）、工具栏、状态栏、面板内阴影、可拖拽分割线 |
| 2026-06-16 | v5.0 | 资产拖拽：缩略图拖拽源 + Inspector Drop Target + AssetDragFormats + SpriteRendererInspector 接入 |

---

## 参考资源

- **ImGui 版本**：`Neverness.Editor.ImGuiFrontend/Views/ContentBrowserImGuiView.cs`
- **Dock 文档**：[AvaloniaFrontend_Dock_Guide.md](AvaloniaFrontend_Dock_Guide.md)
- **框选计划**：[ContentBrowser_RubberBand_Selection.md](ContentBrowser_RubberBand_Selection.md)
