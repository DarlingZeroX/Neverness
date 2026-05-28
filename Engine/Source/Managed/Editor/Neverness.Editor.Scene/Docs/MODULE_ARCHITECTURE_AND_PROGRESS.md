# Neverness.Editor.Scene 模块架构与设计文档

## 目录

- [1. 模块概述](#1-模块概述)
- [2. 文件清单与目录结构](#2-文件清单与目录结构)
- [3. 核心类关系与职责](#3-核心类关系与职责)
- [4. Inspector 系统设计](#4-inspector-系统设计)
- [5. UI 面板体系](#5-ui-面板体系)
- [6. 数据流与事件架构](#6-数据流与事件架构)
- [7. 设计模式总结](#7-设计模式总结)
- [8. 跨模块依赖](#8-跨模块依赖)
- [9. 已知问题与可优化点](#9-已知问题与可优化点)
- [10. 开发进度与待办](#10-开发进度与待办)

---

## 1. 模块概述

`Neverness.Editor.Scene` 是引擎编辑器的 **场景编辑模块**，提供 Unity 风格的场景层级浏览器、组件检查器和编辑器视口。它是 Editor 端场景操作的核心枢纽，桥接了 C# Editor 层与 C++ Native 场景运行时。

**核心职责：**

- 场景层级的可视化浏览与操作（选中、重命名、复制、删除、拖拽重排）
- 实体组件的可视化编辑（Inspector 面板，支持 Transform/Camera/SpriteRenderer）
- 编辑器视口的场景渲染与显示
- Native 场景数据的 Managed 层缓存与版本轮询
- 场景生命周期事件的监听与分发

**构建产物：** .NET 10 类库（`Neverness.Editor.Scene.dll`）

**目标框架：** `net10.0`，启用 unsafe 代码块

**依赖关系：**

| 依赖项目 | 用途 |
|---|---|
| `Neverness.Runtime.Engine` | Native 组件结构体、SceneNativeBridge、API Bootstrap |
| `Neverness.Runtime.Interop` | GUID、AssetHandle、TextureInterop 等互操作类型 |
| `Neverness.Editor.Framework` | IEditorPanel、PanelManager、IEditorEventBus、EditorMenuRegistry |
| `Neverness.Editor.Core` | EditorCoreModule、EditorEvent、EditorEventType、EntityFactory |
| `Neverness.Editor.ImGui` (ThirdParty) | Hexa.NET.ImGui 绑定 |

---

## 2. 文件清单与目录结构

```
Neverness.Editor.Scene/
├── Neverness.Editor.Scene.csproj
├── Public/
│   └── Module.cs                          # 唯一的公共入口（Facade）
├── Private/
│   ├── SceneModuleImp.cs                  # 模块内部实现（编排面板和桥接器）
│   ├── SceneEditorBridge.cs               # SceneManager 事件 → 模块内部路由
│   ├── EditorSceneNativeBridge.cs         # Managed → Native P/Invoke 桥接
│   ├── Cache/
│   │   ├── HierarchyNode.cs               # Native 场景节点的 Managed 表示
│   │   └── SceneHierarchyCache.cs         # 版本轮询的层级快照缓存
│   ├── Debug/
│   │   └── SceneDebug.cs                  # 诊断工具（API 检查、Hex 转储、层级转储）
│   ├── Panel/
│   │   ├── SceneBrowser.cs                # 层级树面板（虚拟化、搜索、拖拽）
│   │   ├── DetailInspector.cs             # 组件检查器面板
│   │   └── EditorViewport.cs              # 3D 场景视口
│   └── Inspector/
│       ├── IComponentInspector.cs         # 接口 + 泛型基类 + InspectorOrderAttribute
│       ├── TransformInspector.cs          # Transform 组件编辑器
│       ├── CameraInspector.cs             # Camera 组件编辑器
│       ├── SpriteRendererInspector.cs     # SpriteRenderer 组件编辑器
│       └── ComponentInspectorRegistry.cs  # 组件检查器注册表（程序集扫描）
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md  # 本文档
└── Test/                                   # 测试目录（待填充）
```

**源文件统计：** 15 个 `.cs` 文件 + 1 个 `.csproj`

---

## 3. 核心类关系与职责

### 3.1 公共入口层

#### SceneModule（Public/Module.cs）

**唯一的公共 API**，静态类，充当 Facade 角色。

```
SceneModule (public static)
├── Install(SceneManager)     # 一次性初始化，注册面板和事件监听
└── SetSceneHandle(ulong)     # 设置当前活动场景句柄
```

所有实现细节隐藏在 `SceneModuleImp` 中。

#### SceneModuleImp（Private/SceneModuleImp.cs）

内部静态类，模块的实际编排中心。

**职责：**
- 创建并注册三个面板（SceneBrowser、DetailInspector、EditorViewport）
- 创建 SceneEditorBridge 并订阅 SceneManager 事件
- 注册编辑器菜单命令（Save Scene）
- 将场景句柄分发到所有面板

### 3.2 桥接层（三层 Bridge 架构）

本模块采用三层 Bridge 模式分离关注点：

#### SceneEditorBridge（Private/SceneEditorBridge.cs）

**Editor → Runtime 事件桥接**

- 实现 `IDisposable`
- 监听 `SceneManager` 的 `SceneActivated` / `SceneUnloaded` 事件
- 将事件路由到 `SceneModuleImp.SetSceneHandle()`

#### EditorSceneNativeBridge（Private/EditorSceneNativeBridge.cs）

**Managed → Native P/Invoke 桥接**

- 静态类，unsafe 上下文
- 封装 `NNEditorSceneApi` 函数表的 P/Invoke 调用
- 提供层级版本查询、快照获取、实体操作等 Native 方法

#### SceneModuleImp（Private/SceneModuleImp.cs）

**模块内部编排**

- 静态类，持有所有面板引用
- 实现 `SetSceneHandle()` 逻辑，将句柄分发到各面板

### 3.3 缓存层

#### HierarchyNode（Private/Cache/HierarchyNode.cs）

Native 场景节点的 Managed 表示，密封类。

```
HierarchyNode (sealed)
├── Handle        # NNEntityHandle
├── Name          # 实体名称
├── ParentIndex   # 父节点索引（-1 = 根节点）
├── IsActive      # 激活状态
└── ChildCount    # 子节点数量
```

**性能优化：** 数组槽位跨帧复用，仅在节点数量变化时重新分配。

#### SceneHierarchyCache（Private/Cache/SceneHierarchyCache.cs）

版本轮询缓存，密封类。

```
SceneHierarchyCache (sealed)
├── m_Version           # 上次快照的版本号
├── m_Nodes             # HierarchyNode[] 缓存
├── m_SelectedEntities  # 选中实体集合
├── TryRefresh(handle)  # 版本轮询 → 按需拉取完整快照
├── Select(handle)      # 选中实体
└── ClearSelection()    # 清除选中
```

**核心机制 — 版本轮询：**
1. 每帧调用 `EditorSceneNativeBridge.GetHierarchyVersion()`（1 次 P/Invoke，整数返回）
2. 若版本未变 → 跳过，零开销
3. 若版本变化 → 调用 `GetSnapshotSize()` + `GetHierarchySnapshot()` 拉取完整二进制快照
4. 解析快照 → 更新 `HierarchyNode[]`

这种设计将 P/Invoke 开销从 O(n) 降到 O(1)（通常情况）。

### 3.4 调试工具

#### SceneDebug（Private/Debug/SceneDebug.cs）

静态诊断工具类，提供：
- `CheckApiIntegrity()` — 验证 Native API 函数表完整性
- `HexDump()` — 二进制数据的十六进制转储
- `DumpHierarchy()` — 层级结构的可读化输出

---

## 4. Inspector 系统设计

Inspector 系统采用 **Strategy + Registry + Template Method** 三重模式组合。

### 4.1 接口与基类

#### IComponentInspector（接口）

```csharp
public interface IComponentInspector
{
    ulong ComponentTypeId { get; }       // Native 组件类型 ID
    string DisplayName { get; }          // UI 显示名称
    Type ClrType { get; }                // CLR 类型
    int Order { get; }                   // 显示顺序
    bool HasComponent(ulong sceneHandle, NNEntityHandle entity);
    void RemoveComponent(ulong sceneHandle, NNEntityHandle entity);
    bool DrawInspector(ulong sceneHandle, NNEntityHandle entity);  // 返回是否修改
}
```

#### ComponentTypeInspector\<T\>（泛型抽象基类）

```csharp
public abstract class ComponentTypeInspector<T> : IComponentInspector
    where T : unmanaged
{
    static readonly ulong s_typeId;      // 从 ComponentIdAttribute 解析
    static readonly string s_displayName; // 从 ComponentIdAttribute 解析

    bool DrawInspector(sceneHandle, entity)
    {
        // Template Method：读取 → 绘制 → 写回
        var data = SceneNativeBridge.GetComponent<T>(sceneHandle, entity);
        bool modified = DrawFields(ref data);
        if (modified) SceneNativeBridge.SetComponent(sceneHandle, entity, ref data);
        return modified;
    }

    protected abstract bool DrawFields(ref T data);  // 子类实现
}
```

#### InspectorOrderAttribute

控制检查器在 UI 中的显示顺序（值越小越靠前）。

### 4.2 已注册的 Inspector

| Inspector | 组件类型 | Order | 功能 |
|---|---|---|---|
| `TransformInspector` | `NNTransformData` | 0 | Position/Rotation/Scale（欧拉角转换） |
| `SpriteRendererInspector` | `NNSpriteRendererComponentData` | 50 | 纹理拖放区、颜色、UV、混合模式、标志位 |
| `CameraInspector` | `NNCameraComponentData` | 100 | 投影类型、FOV、裁剪平面、投影矩阵查看器 |

### 4.3 注册机制（ComponentInspectorRegistry）

```csharp
public static class ComponentInspectorRegistry
{
    static bool s_Discovered;
    static readonly List<IComponentInspector> s_Inspectors;

    // 懒加载：首次访问时扫描程序集
    public static IReadOnlyList<IComponentInspector> Inspectors
    {
        get
        {
            if (!s_Discovered) DiscoverFromAssembly(...);
            return s_Inspectors;
        }
    }

    // 手动注册也支持
    public static void Register(IComponentInspector inspector);
}
```

**扫描逻辑：** 首次访问时扫描 `Neverness.Editor.Scene` 程序集中所有非抽象的 `IComponentInspector` 实现，按 `Order` 排序。

### 4.4 SpriteRendererInspector 详细设计

`NNSpriteRendererComponentData` 内存布局（88 字节，blittable）：

| 字段 | 类型 | 偏移 | Inspector 控件 |
|---|---|---|---|
| `TextureAsset` | `NNGuid` (16B) | 0 | 128x128 拖放预览区 |
| `MaterialAsset` | `NNGuid` (16B) | 16 | 只读 GUID / "Default" |
| `TextureRuntimeId` | `uint` | 32 | 不可编辑（运行时缓存） |
| `ColorR/G/B/A` | `float` x4 | 36 | `ImGui.ColorEdit4` |
| `UvU0/V0/U1/V1` | `float` x4 | 52 | `ImGui.DragFloat` x4 |
| `Layer` | `uint` | 68 | `ImGui.InputInt` |
| `SortOrder` | `uint` | 72 | `ImGui.InputInt` |
| `BlendMode` | `uint` | 76 | `ImGui.Combo`（5 种混合模式） |
| `Flags` | `uint` | 80 | 复选框（Visible/FlipX/FlipY） |

**纹理拖放区工作流程：**
1. 渲染 128x128 `InvisibleButton` 作为 drop zone
2. 若有纹理 GUID → 通过 `AssetHandleExtensions.LoadSync()` + `TextureInterop.LoadTextureFromBlob()` 加载预览
3. 接受 `TEXTURE_ASSET` 类型的拖放 payload（128-bit GUID）
4. 右键点击清除纹理

---

## 5. UI 面板体系

所有面板实现 `IEditorPanel` 接口（来自 `Neverness.Editor.Framework`），在 `SceneModuleImp.Install()` 中注册到 `PanelManager`。

### 5.1 SceneBrowser — 层级树浏览器

**功能矩阵：**

| 功能 | 实现方式 |
|---|---|
| 树形显示 | `ImGui.TreeNodeEx` + DFS 深度缩进 |
| 虚拟化 | 手动虚拟化：根据滚动位置计算可见行范围，仅渲染可见节点 |
| 搜索过滤 | 工具栏搜索框，按名称过滤 |
| 选中 | 点击节点 → `SceneHierarchyCache.Select()` → 发射 `SelectionChanged` 事件 |
| 重命名 | 右键菜单 → Modal Popup + `InputText` |
| 复制 | 右键菜单 → 复制 `NNTransformData` 和 `NNCameraComponentData` |
| 删除 | 右键菜单 → `SceneNativeBridge.DestroyEntity()` |
| 拖拽重排 | `ImGui.BeginDragDropSource/Target`，payload 类型 `ENTITY` |
| 添加实体 | 背景右键菜单 → "Add Entity" 子菜单（Camera / Sprite），使用 `EntityFactory` |
| 展开/折叠全部 | 工具栏按钮 |
| 节点计数 | 工具栏显示 |

### 5.2 DetailInspector — 组件检查器

**功能矩阵：**

| 功能 | 实现方式 |
|---|---|
| 选中状态 | 监听 `SelectionChanged`/`SceneClosed` 事件 + 轮询 `SceneHierarchyCache.SelectedEntities` |
| 空状态 | 居中提示文字（无选中实体时） |
| 实体头 | 名称、十六进制句柄、Active 复选框 |
| 组件分组 | `CollapsingHeader`（来自 `ComponentInspectorRegistry`） |
| 删除组件 | 右键组件头 → "Remove Component" |
| 添加组件 | 底部 "Add Component" 按钮 → Popup 列出未添加的组件 |

### 5.3 EditorViewport — 场景视口

**工作机制：**
1. 调用 `EngineNativeApiBootstrap.EngineApi.ViewportRender.RenderSceneToTexture(sceneHandle, width, height)` 获取 GPU 纹理 ID
2. 通过 `ImGui.Image` 显示
3. UV 翻转（OpenGL 原点在左下角，ImGui 期望左上角）
4. 零内边距最大化视口区域

---

## 6. 数据流与事件架构

### 6.1 场景激活流程

```
SceneManager.SceneActivated 事件
    │
    ▼
SceneEditorBridge（监听 SceneManager 事件）
    │
    ▼
SceneModuleImp.SetSceneHandle(handle)
    │
    ├── SceneBrowser.SceneHandle = handle
    ├── DetailInspector.SceneHandle = handle
    └── EditorViewport.SetScene(handle)
```

### 6.2 层级刷新流程（每帧）

```
SceneBrowser.OnUpdate(delta)
    │
    ▼
SceneHierarchyCache.TryRefresh(sceneHandle)
    │
    ├── EditorSceneNativeBridge.GetHierarchyVersion()     ← 1 次 P/Invoke（整数）
    │
    ├── 版本未变 → 跳过（O(1) 开销）
    │
    └── 版本变化：
        ├── EditorSceneNativeBridge.GetSnapshotSize()     ← 获取快照大小
        ├── EditorSceneNativeBridge.GetHierarchySnapshot() ← 批量二进制拷贝
        └── ParseSnapshot() → HierarchyNode[]             ← Managed 层解析
```

### 6.3 选中与编辑流程

```
用户点击 SceneBrowser 节点
    │
    ▼
SceneHierarchyCache.Select(entityHandle)
    │
    ▼
IEditorEventBus.Emit(SelectionChanged, entityHandle)
    │
    ▼
DetailInspector.OnSelectionChanged(event)
    │
    ▼
SetSelectedEntity(handle)
    │
    ▼
DrawComponents()
    │
    ├── ComponentInspectorRegistry.Inspectors
    │       │
    │       ├── HasComponent(handle, entity) 过滤
    │       │
    │       └── DrawInspector(handle, entity)
    │               │
    │               └── ComponentTypeInspector<T>.DrawInspector()
    │                       │
    │                       ├── SceneNativeBridge.GetComponent<T>()   ← 读取
    │                       ├── DrawFields(ref T)                      ← UI 绘制
    │                       └── SceneNativeBridge.SetComponent()       ← 写回（若修改）
    │
    └── "Add Component" Popup → SceneNativeBridge.AddComponent(handle, entity, typeId)
```

### 6.4 场景保存流程

```
EditorMenuRegistry → EditorCommand("Save Scene")
    │
    ▼
Execute 回调：
    │
    ├── SceneModuleImp.CurrentSceneWorld != null?
    ├── SceneManager.SaveScene(sceneWorld)
    └── EditorState.CurrentScenePath 更新
```

---

## 7. 设计模式总结

| 模式 | 位置 | 说明 |
|---|---|---|
| **Template Method** | `ComponentTypeInspector<T>` | 基类实现 read-modify-write 循环；子类覆盖 `DrawFields()` |
| **Strategy** | `IComponentInspector` | 每种组件类型是一种检查器渲染策略 |
| **Registry** | `ComponentInspectorRegistry` | 集中注册 + 懒加载程序集扫描 + 有序列表 |
| **Observer / Event Bus** | `IEditorEventBus` | `SceneBrowser` 发射 `SelectionChanged`；`DetailInspector` 订阅 |
| **Bridge** | 三层桥接 | Editor↔Runtime 事件桥、Managed↔Native P/Invoke 桥、模块内部编排桥 |
| **Facade** | `SceneModule` | 单一公共静态类，隐藏所有私有实现 |
| **Snapshot / Version Polling** | `SceneHierarchyCache` | 每帧轮询版本号，版本变化才拉取完整快照，最小化 P/Invoke 开销 |
| **Command** | `EditorCommand` (Save Scene) | 通过 `EditorMenuRegistry.RegisterCommand()` 注册菜单命令 |
| **Flyweight-like** | `HierarchyNode` | 跨帧复用数组槽位，仅在节点数变化时重新分配 |
| **Attribute-based Discovery** | `InspectorOrderAttribute`, `ComponentIdAttribute` | 特性驱动排序和类型 ID 解析，无硬编码 |

---

## 8. 跨模块依赖

### 8.1 Neverness.Runtime.Engine

| 类型 / 类 | 用途 |
|---|---|
| `NNEntityHandle` | 实体句柄 |
| `NNTransformData`, `NNCameraComponentData`, `NNSpriteRendererComponentData` | Native 组件结构体（blittable） |
| `SceneNativeBridge` | 泛型 Has/Get/Set/Remove/Add Component 方法 |
| `EngineNativeApiBootstrap` | 访问 `NNEngineApi` 函数表 |
| `NNSceneSnapshotHeader`, `NNSceneNodeSnapshot` | 层级快照数据结构 |
| `NNProjectionType`, `NNSpriteFlags`, `NNMat4`, `NNVec3`, `NNQuat`, `NNGuid` | 值类型 |
| `ComponentIdAttribute` | 解析组件 TypeId 和显示名称 |
| `NNEditorComponentInfo`, `NNEditorFieldInfo` | Editor 端组件/字段元数据 |

### 8.2 Neverness.Runtime.Interop

| 类型 | 用途 |
|---|---|
| `GUID` | 资产标识 |
| `AssetHandleExtensions.LoadSync()` | 同步加载资产 |
| `TextureInterop` | 纹理加载和 ImGui 句柄获取 |

### 8.3 Neverness.Editor.Framework

| 类型 | 用途 |
|---|---|
| `IEditorPanel`, `IPanel` | UI 面板接口契约 |
| `PanelManager` | 单例面板注册中心 |
| `IEditorEventBus`, `EditorEvent`, `EditorEventType` | 事件总线 |
| `EditorMenuRegistry`, `EditorMenuItem`, `EditorCommand` | 菜单/命令系统 |
| `FontAwesome5Pro` | 窗口标题图标常量 |

### 8.4 Neverness.Editor.Core

| 类型 | 用途 |
|---|---|
| `EditorCoreModule.Context` | 访问事件总线和编辑器状态 |
| `EditorState` | 追踪 `CurrentScenePath`、`CurrentSceneHandle` |
| `EntityFactory` | 创建 Camera/Sprite 实体（SceneBrowser 右键菜单使用） |

---

## 9. 已知问题与可优化点

### 9.1 Duplicate 命令硬编码组件类型（高优先级）

**现状：** `SceneBrowser` 的 Duplicate 功能仅复制 `NNTransformData` 和 `NNCameraComponentData`，硬编码了组件列表。

**问题：** 新增组件类型（如未来添加 Light、RigidBody）时，必须手动更新 Duplicate 逻辑。

**优化方案：** 使用 `ComponentInspectorRegistry` 遍历所有注册的组件类型，通过反射/泛型逐一复制，或在 Native 端实现通用的 `CloneEntity()`。

### 9.2 SpriteRendererInspector 纹理预览每帧重建（中优先级）

**现状：** `DrawFields()` 中纹理预览的 ImGui 句柄查询（`TextureInterop.GetImGuiTextureHandle()`）在每帧绘制时调用。

**优化方案：** 缓存 ImGui 句柄，仅在 `TextureAsset` GUID 变化时重新查询。可增加 dirty flag 或 GUID 对比。

### 9.3 SceneHierarchyCache 的 TryRefresh 每帧全量轮询（中优先级）

**现状：** `TryRefresh()` 在每帧的 `SceneBrowser.OnUpdate()` 中调用，即使场景未发生任何变化。

**优化方向：**
- 降低轮询频率（如每 N 帧检查一次，或使用脏标记）
- 增量快照（仅传输变化的节点，而非全量二进制快照）

### 9.4 DetailInspector 双重选中同步机制（中优先级）

**现状：** `DetailInspector` 同时使用事件订阅（`SelectionChanged`）和主动轮询（`SyncSelection()`）来同步选中状态。

**问题：** 双重机制增加了复杂度和潜在的不一致风险。

**优化方案：** 统一为纯事件驱动，移除 `SyncSelection()` 轮询。确保所有选中变更路径都通过 `IEditorEventBus`。

### 9.5 缺少 Undo/Redo 支持（低优先级）

**现状：** 所有组件编辑（SetComponent）直接写入 Native 内存，无撤销/重做机制。

**优化方案：**
- 实现 Command Pattern 的 Undo/Redo 栈
- `DrawFields()` 修改前记录旧值 → 生成 Undo 命令
- 与 `EditorMenuRegistry` 的 Edit 菜单集成

### 9.6 EditorViewport 缺少交互能力（低优先级）

**现状：** 视口仅显示渲染结果，不支持相机控制（平移/旋转/缩放）、实体拾取（点击选中）、Gizmo 操作。

**优化方案：**
- 实现鼠标输入处理（右键拖拽旋转、滚轮缩放、中键平移）
- 实现射线拾取（从屏幕坐标到实体）
- 集成 ImGuizmo 或自定义 Gizmo 渲染

### 9.7 程序集扫描仅限自身程序集（低优先级）

**现状：** `ComponentInspectorRegistry.DiscoverFromAssembly()` 仅扫描 `Neverness.Editor.Scene` 程序集。

**问题：** 如果其他模块（如 Editor.Plugins）定义了新的组件 Inspector，无法自动发现。

**优化方案：** 扫描所有已加载程序集，或提供显式的跨程序集注册 API。

### 9.8 SceneBrowser 虚拟化实现较脆弱（低优先级）

**现状：** 手动虚拟化通过滚动位置数学计算可见行范围，在节点高度不一致或展开/折叠动画期间可能产生闪烁。

**优化方案：** 使用更成熟的虚拟化方案（如 ImGui.Clipper），或维护节点高度缓存。

### 9.9 缺少单元测试（低优先级）

**现状：** `Test/` 目录为空。

**建议覆盖：**
- `SceneHierarchyCache` 的版本轮询和快照解析正确性
- `ComponentInspectorRegistry` 的程序集扫描和排序
- `HierarchyNode` 的内存布局与 Native 结构体一致性验证

---

## 10. 开发进度与待办

### 已完成

- [x] 模块 Public/Private 分层架构（Facade 模式）
- [x] 三层 Bridge 架构（Editor↔Runtime 事件桥、P/Invoke 桥、内部编排桥）
- [x] SceneBrowser 层级树面板（虚拟化、搜索、拖拽、右键菜单）
- [x] DetailInspector 组件检查器面板（折叠分组、添加/删除组件）
- [x] EditorViewport 场景渲染视口
- [x] Inspector 系统框架（IComponentInspector + ComponentTypeInspector\<T\> + Registry）
- [x] TransformInspector（Position/Rotation/Scale，欧拉角）
- [x] CameraInspector（投影类型、FOV、裁剪平面、矩阵查看器）
- [x] SpriteRendererInspector（纹理拖放、颜色、UV、混合模式、标志位）
- [x] SceneHierarchyCache 版本轮询缓存
- [x] 组件检查器的程序集扫描自动发现
- [x] 场景保存命令（EditorCommand + EditorMenuRegistry）
- [x] 实体操作（重命名、复制、删除、拖拽重排）

### 待实现

- [ ] Undo/Redo 撤销重做系统
- [ ] EditorViewport 交互（相机控制、实体拾取、Gizmo）
- [ ] Duplicate 命令的通用组件复制（不硬编码）
- [ ] 纹理预览 ImGui 句柄缓存
- [ ] 增量层级快照（减少 P/Invoke 数据传输）
- [ ] 跨程序集 Inspector 注册支持
- [ ] SceneBrowser 虚拟化方案升级
- [ ] 单元测试

---

*最后更新：2026/05/28*
