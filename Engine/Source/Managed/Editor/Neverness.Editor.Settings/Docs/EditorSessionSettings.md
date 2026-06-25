# EditorSessionSettings 实施文档

**日期**：2026-06-24
**模块**：Neverness.Editor.Settings
**Scope**：Project（每个项目独立保存）

---

## 概述

新增 `EditorSessionSettings` 设置表，用于持久化编辑器会话状态（上次打开的场景、选中的实体等）。编辑器启动时自动恢复上次打开的场景，无需用户手动重新打开。

## 设计决策

### 为什么独立成 Session 表？

| 方案 | 优点 | 缺点 |
|---|---|---|
| 加入 EditorPreferencesSettings | 改动最小 | 偏好设置是 User Scope，会话是 Project Scope，语义混杂 |
| **独立 EditorSessionSettings** ✅ | Scope 分离清晰，可扩展 | 多注册一个 SettingsTable |
| 沿用 Launcher 的 ProjectInfo.DefaultScene | 不引入新表 | Editor 依赖 Launcher 模型，耦合 |
| .lastscene 标记文件 | 极简 | 不成体系，与设置系统脱节 |

### Scope = Project 的原因

- 会话状态（打开的场景、窗口布局）与项目绑定，不同项目有不同的"上次场景"
- 偏好设置（字体、IDE 选择）与用户绑定，跨项目共享

---

## 文件清单

### 新建文件

| # | 文件完整路径 | 说明 |
|---|---|---|
| 1 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Private/Descriptors/EditorSessionSettings.cs` | SettingsTable 子类，`LastOpenedScene` + `LastSelectedEntityHandle`，使用 `SetProperty()` |
| 2 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Private/Descriptors/EditorSessionSettingsDescriptor.cs` | ISettingsDescriptor 手写实现 |

### 修改文件

| # | 文件完整路径 | 改动说明 |
|---|---|---|
| 1 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Public/EditorSettings.cs` | 新增 `Session` 静态属性（第 29 行） |
| 2 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Public/SettingsModule.cs` | 注册 + 加载 `editorSession` + PropertyChanged 自动持久化（第 52-75 行） |
| 3 | `Engine/Source/Managed/Editor/Neverness.Editor.Assets/AssetOpening/SceneAssetOpener.cs` | 打开场景成功后写 `EditorSettings.Session.LastOpenedScene`（第 36 行） |
| 4 | `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/SceneModuleImp.cs` | Save 命令成功后同步更新 Session（第 132 行） |
| 5 | `Engine/Source/Managed/Editor/NevernessEditor/EditorApplicationRunner.cs` | 新增 `AutoLoadLastScene()` 方法 + Phase 6 调用（第 260-284 行） |
| 6 | `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Neverness.Editor.Scene.csproj` | 新增 Settings ProjectReference（第 25 行） |

## 改动详情

### 1. EditorSessionSettings.cs（新建）

SettingsTable 子类，定义两个字段：

```csharp
[SettingTable("editorSession", "会话状态", Scope = SettingsScope.Project, Category = "编辑器")]
public sealed class EditorSessionSettings : SettingsTable
{
    // 使用 SetProperty() 触发 PropertyChanged，驱动自动持久化
    public string? LastOpenedScene { get; set; }      // VFS 路径
    public ulong LastSelectedEntityHandle { get; set; } // 实体句柄
}
```

关键点：
- **必须用 `SetProperty()` 而非 auto-property**，否则 `PropertyChanged` 不触发，自动保存不生效
- `LastOpenedScene` 使用 VFS 路径（如 `/assets/scenes/main.scene`），不使用磁盘路径
- `LastSelectedEntityHandle` 预留，当前未接入（需 EditorState 双向同步）

### 2. EditorSessionSettingsDescriptor.cs（新建）

手写 ISettingsDescriptor，Phase 2 由 Source Generator 替代。

### 3. EditorSettings.cs（修改）

新增静态属性：

```csharp
public static EditorSessionSettings Session { get; } = new();
```

### 4. SettingsModule.cs（修改）

在 `Install()` 中新增三处逻辑：

```csharp
// 注册
service.Register(EditorSettings.Session, EditorSessionSettingsDescriptor.Instance);

// 加载
service.Load("editorSession");

// PropertyChanged → 自动持久化
EditorSettings.Session.PropertyChanged += (_, e) =>
{
    if (!string.IsNullOrEmpty(e.PropertyName))
    {
        EditorSettings.RaiseSettingChanged("editorSession", e.PropertyName);
        service.Save("editorSession");  // 变更时立即写盘
    }
};
```

### 5. SceneAssetOpener.cs（修改）

打开场景成功后记录路径：

```csharp
EditorSettings.Session.LastOpenedScene = vfsPath;
```

### 6. SceneModuleImp.cs（修改）

Save 命令成功后同步更新（覆盖 SaveAs 后路径变化的情况）：

```csharp
var assetPath = sceneManager.ActiveWorld?.AssetPath;
if (!string.IsNullOrEmpty(assetPath))
    EditorSettings.Session.LastOpenedScene = assetPath;
```

### 7. EditorApplicationRunner.cs（修改）

Phase 6 新增 `AutoLoadLastScene()`：

```csharp
private static void AutoLoadLastScene(SceneManager sceneManager)
{
    var lastScene = EditorSettings.Session.LastOpenedScene;
    if (string.IsNullOrEmpty(lastScene)) return;

    var sceneName = Path.GetFileNameWithoutExtension(lastScene);
    var result = sceneManager.LoadSceneFromAsset(sceneName, lastScene);
    if (!result)
        EditorSettings.Session.LastOpenedScene = null; // 清除无效记录
}
```

### 8. Neverness.Editor.Scene.csproj（修改）

新增 ProjectReference：

```xml
<ProjectReference Include="..\Neverness.Editor.Settings\Neverness.Editor.Settings.csproj" />
```

---

## 数据流

```
┌─ 打开场景 ─────────────────────────────────────────────────────┐
│  SceneAssetOpener.OpenAsync()                                  │
│    → SceneManager.LoadSceneFromAsset(name, vfsPath)            │
│    → EditorSettings.Session.LastOpenedScene = vfsPath          │
│         ↓ SetProperty() → PropertyChanged                     │
│         ↓ SettingsModule 自动 Save("editorSession")            │
│         ↓ SettingsStorage → /projectSettings/Settings/         │
│                                editorSession.json              │
└────────────────────────────────────────────────────────────────┘

┌─ 启动编辑器 ───────────────────────────────────────────────────┐
│  SettingsModule.Install()                                      │
│    → service.Load("editorSession")                             │
│    → EditorSettings.Session 从 JSON 填充                        │
│                                                                │
│  EditorApplicationRunner.Install() Phase 6                     │
│    → AutoLoadLastScene(sceneManager)                           │
│    → sceneManager.LoadSceneFromAsset(name, lastScene)          │
│    → 加载失败则清除 LastOpenedScene                              │
└────────────────────────────────────────────────────────────────┘
```

## 存储格式

文件路径：`/projectSettings/Settings/editorSession.json`

```json
{
  "lastOpenedScene": "/assets/scenes/main.scene",
  "lastSelectedEntityHandle": 0
}
```

- `null` 值的字段不会写入 JSON（`DefaultIgnoreCondition = WhenWritingNull`）
- 字段名为 camelCase（`JsonNamingPolicy.CamelCase`）

## 扩展点

未来可在 `EditorSessionSettings` 中新增字段：

| 字段 | 类型 | 用途 |
|---|---|---|
| `WindowLayout` | `string?` | DockPanel 布局 XML/JSON |
| `CameraPosition` | `string?` | 视口相机位置（序列化的 Vector3） |
| `OpenAssetTabs` | `string?` | 当前打开的资产标签页列表 |
| `ZoomLevel` | `float` | 内容浏览器缩放级别 |

每个新字段只需：
1. 在 `EditorSessionSettings` 中添加属性（用 `SetProperty()`）
2. 在 `EditorSessionSettingsDescriptor` 中添加 `FieldDescriptor`
3. 在对应业务代码中读写该字段

## 注意事项

- `EditorPreferencesSettings` 使用 auto-property，**不会**触发 PropertyChanged，需手动调 `Save("editorPreferences")` 持久化
- `EditorSessionSettings` 使用 `SetProperty()`，变更时**自动**写盘
- 这两种模式是刻意的：偏好设置变更频繁（如拖动滑块），需节流；会话状态变更稀疏（如打开场景），立即保存无性能问题
