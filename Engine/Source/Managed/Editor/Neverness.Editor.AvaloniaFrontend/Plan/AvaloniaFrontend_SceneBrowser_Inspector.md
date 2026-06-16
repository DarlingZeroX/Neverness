# SceneBrowser + Inspector Avalonia 实施记录

> 状态：基础功能已完成（2026-06-13）

---

## 架构

```
SceneBrowserAvaloniaView → SceneBrowserViewModel → SceneBrowserController → ISceneQueryService
                                                            ↓ (PropertyChanged 桥接)
InspectorAvaloniaView ← InspectorViewModel ← EditorCompositionRoot ← ISceneQueryService
```

桥接方式：`EditorCompositionRoot.ConnectSceneBrowserToInspector()` 订阅 `SceneBrowserVM.PropertyChanged`，当 `SelectedEntityHandle` 变更时直接用 `ISceneQueryService` 获取实体信息并更新 `InspectorVM`。

---

## 已实现功能

### SceneBrowser
- TreeView 显示实体层级树
- 单击选中实体（`TreeView.SelectionChanged` 事件）
- 展开/折叠状态持久化（`_viewModel.SetExpanded`）
- 搜索框过滤（`_viewModel.SearchText`）
- 展开全部 / 折叠全部按钮
- 节点计数显示

### Inspector
- 显示选中实体名称
- 显示激活状态 CheckBox
- 显示组件列表（通过原生桥 `EditorSceneNativeBridge.GetEntityComponents`）
- Add Component 按钮（TODO: 弹窗）
- 无选中时显示 "No entity selected"

### SceneBrowser → Inspector 桥接
- `SceneBrowserVM.PropertyChanged` → `SelectedEntityHandle` 变更
- 直接用 `ISceneQueryService.GetEntityName(handle)` 获取实体名
- 直接用 `ISceneQueryService.GetEntity(handle)` 获取激活状态
- 更新 `InspectorVM` + 调用 `InspectorController.RefreshComponents()`

---

## 关键文件

| 文件 | 职责 |
|------|------|
| `Views/SceneBrowserAvaloniaView.cs` | SceneBrowser 视图 |
| `Views/InspectorAvaloniaView.cs` | Inspector 视图 |
| `ViewModels/SceneBrowserViewModel.cs` | 实体树状态 |
| `ViewModels/InspectorViewModel.cs` | 组件列表状态 |
| `Controllers/SceneBrowserController.cs` | 实体树操作 |
| `Controllers/InspectorController.cs` | 组件操作 |
| `Public/EditorCompositionRoot.cs` | SceneBrowser↔Inspector 桥接 |
| `Inspectors/AvaloniaComponentInspectorRegistry.cs` | Avalonia Inspector 注册表 |
| `Inspectors/TransformInspector.cs` | Transform 组件 Inspector（TypeId 占位） |
| `Inspectors/CameraInspector.cs` | Camera 组件 Inspector（TypeId 占位） |
| `Inspectors/SpriteRendererInspector.cs` | SpriteRenderer 组件 Inspector（TypeId 占位） |

---

## 踩坑记录

### 问题 1：TreeViewItem.DoubleTapped 不触发

**现象**：给 `TreeViewItem` 绑定 `DoubleTapped` 事件，点击不触发。

**根因**：`TreeViewItem` 的点击被展开/折叠逻辑消耗。

**修复**：改用 `TreeView.SelectionChanged` 事件，单击即触发。

### 问题 2：Inspector 实体名为空

**现象**：点击实体后 Inspector 显示空名称。

**根因**：`InspectorServiceImpl.GetEntityName` 用 `EditorCoreModule.Context` 获取 `ISceneQueryService`，与 `SceneBrowserController` 注入的实例不同，缓存不同步。

**修复**：`ConnectSceneBrowserToInspector` 桥接中直接用 `ISceneQueryService.GetEntityName()` 获取实体名（与 SceneBrowser 同源），绕过 `InspectorService`。

### 问题 3：Inspector 组件列表为空

**现象**：点击实体后 Inspector 不显示任何组件。

**根因**：`InspectorServiceImpl.GetEntityComponents` 用 `ComponentInspectorRegistry.Inspectors`（ImGui 注册表）遍历检测组件，该注册表可能为空。

**修复**：改用原生桥 `EditorSceneNativeBridge.GetEntityComponents()` 直接查询组件，不再依赖 ImGui 注册表。

### 问题 4：Avalonia Inspector 未注册

**现象**：组件检测正常但显示 "No inspector for component type"。

**根因**：`AvaloniaComponentInspectorRegistry.DiscoverFromAssembly()` 未调用。

**修复**：在 `AvaloniaFrontendModule.Install()` 中调用 `DiscoverFromAssembly` 扫描 Inspector 实现。

### 问题 5：Inspector TypeId 是占位符（已修复）

**现象**：Avalonia Inspector 的 `CanInspect(typeId)` 始终返回 false。

**根因**：`TransformInspector` 等使用 `TransformTypeId = 1` 占位，与原生引擎实际 TypeId 不匹配。

**修复**：从 `ComponentIdAttribute` 获取实际 TypeId：
- Transform: `0xC1FFF4F356DFB2FB`
- Camera: `0x54D1B2A64667E32E`
- SpriteRenderer: `0x51387BA3968C343B`

### 问题 6：Inspector 组件列表位置错误（已修复）

**现象**：Add Component 按钮位置不对。

**根因**：DockPanel 子控件添加顺序错误，ScrollViewer 在 Bottom 之前添加导致布局异常。

**修复**：调整顺序——先添加 Top 和 Bottom 停靠的控件，最后添加 ScrollViewer 填充剩余空间。

---

## Avalonia Inspector 注册表机制

```csharp
// 启动时扫描程序集
AvaloniaComponentInspectorRegistry.DiscoverFromAssembly(typeof(AvaloniaFrontendModule).Assembly);

// 运行时创建 Inspector UI
var inspectorUI = AvaloniaComponentInspectorRegistry.CreateInspectorUI(sceneHandle, entityHandle, typeId);
// → 先查找注册的 Inspector（CanInspect 匹配）
// → 未找到则返回通用 Inspector（显示 TypeId）
```

---

## Inspector UI 组件

### DragFloat 控件
自定义控件，类似 ImGui 的 DragFloat 行为：
- 鼠标左键拖动：左右拖动调整数值
- 双击：进入编辑模式，直接输入数值
- Enter 确认，Esc 取消
- 毛玻璃半透明背景
- 点击任何非 TextBox 区域自动退出编辑模式

### Inspector 布局
- **Transform**：Position/Rotation/Xyz 并排输入，X/Y/Z 按钮点击重置（Position/Rotation 归零，Scale 置 1）
- **Camera**：Projection/FOV/Near/Far/Aspect 属性行
- **SpriteRenderer**：Texture 区域 + Color RGBA 并排输入 + Layer/Sort Order/Blend Mode

---

## 待实现功能

1. **Inspector 字段编辑**：实际读写组件字段数据（Position/Rotation/Scale 等）
2. **Add Component 弹窗**：显示可用组件类型列表
3. **右键菜单**：Rename、Delete、Duplicate、Create Child
4. **拖拽重设父节点**
5. **多选支持**
6. **SceneBrowser 自动刷新**：场景变更时自动更新树
7. **更多 Inspector**：AudioSource、VideoPlayer、Script、RmlUIDocument

---

## 参考资源

- **ImGui 版本**：`Neverness.Editor.ImGuiFrontend/Views/SceneBrowserImGuiView.cs`
- **ImGui Inspector 注册表**：`Neverness.Editor.Scene/Private/ComponentInspectorRegistry.cs`
- **原生桥**：`Neverness.Editor.Scene/Private/EditorSceneNativeBridge.cs`
