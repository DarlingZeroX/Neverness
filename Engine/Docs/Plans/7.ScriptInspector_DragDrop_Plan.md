# ScriptInspector + ContentBrowser 拖拽绑定 — 实施计划 v2

> **日期**: 2026-06-02
> **状态**: ✅ 批准实施
> **模块**: Neverness.Editor.Scene + Neverness.Editor.Script + Neverness.Editor.Framework

---

## 1. 目标

1. DetailInspector 的 "+ Add Component" 菜单中可以添加 Script 组件
2. ContentBrowser 中的 `.cs` 脚本资产可以拖拽到 DetailInspector 绑定到实体

---

## 2. 架构分层

```
ECS 层（Native + Runtime ABI）
  NNScriptComponent { ScriptTypeId, Enabled }
  可序列化、可 Prefab、无运行时引用

Editor 层
  ScriptInspector — UI 渲染 + 拖拽接收
  ScriptAssetIndex — GUID ↔ FullName ↔ ScriptTypeId 映射缓存
  AssetDragDrop — 拖拽协议扩展

Runtime 层
  ScriptRegistry — TypeId ↔ Type 映射（唯一权威）
  ScriptBehaviourScheduler — Behaviour 生命周期
  BehaviourRegistry — Entity ↔ Behaviour 映射
```

---

## 3. 核心设计规则

### 规则 1：ScriptRegistry 是 ScriptTypeId 的唯一权威来源

```
❌ Editor 源文件解析 → 生成 ScriptTypeId（不可靠）
✅ ScriptRegistry 已注册的 Type → ScriptTypeId（权威）

拖拽流程：
  .cs GUID → ScriptAssetIndex 查 FullName
    → ScriptRegistry.TryFindByFullName(fullName)
      ├─ 成功 → 使用 Registry 中的 TypeId（唯一正确来源）
      └─ 失败 → fallback FNV1a64(FullName)（脚本未编译时的临时值）
```

### 规则 2：ScriptAssetIndex 是 GUID ↔ FullName 的唯一映射

```
Editor 启动 / 脚本编译成功时构建：
  ScriptAssetIndex
    ├── GUID → FullName（从 .cs 源文件解析，一次性构建）
    ├── FullName → ScriptTypeId（从 ScriptRegistry 查询）
    └── GUID → ScriptTypeId（组合查询）
```

### 规则 3：ScriptTypeId 只能来源于 ScriptRegistry（硬规则）

```
ScriptTypeId 只能来源于 ScriptRegistry（运行时权威）
Editor 不得直接计算最终 TypeId（允许辅助，但不提交结果）
```

### 规则 4：Scheduler 不反向修改 ECS

```
硬规则：
  - ScriptComponent 变更 → 只影响下一帧 Scheduler diff
  - Scheduler 永远不反向写入 ECS
  - Behaviour 生命周期 ≠ ECS Component 生命周期

后果：
  - ECS Disable → Scheduler 下一帧停止调用 OnUpdate
  - ECS Remove Component → Scheduler 下一帧销毁 Behaviour
  - ECS Replace ScriptTypeId → Scheduler 下一帧销毁旧 Behaviour + 创建新 Behaviour
```

---

## 4. ScriptAssetIndex 设计

### 4.1 数据结构

```csharp
/// <summary>
/// 脚本资产索引——Editor 层缓存，GUID ↔ FullName ↔ ScriptTypeId 映射。
/// 构建时机：Editor 启动扫描 + 脚本编译成功回调。
/// </summary>
public sealed class ScriptAssetIndex
{
    // GUID → FullName（从 .cs 源文件解析，一次性构建）
    private readonly Dictionary<Guid, string> _guidToFullName = new();

    // FullName → ScriptTypeId（从 ScriptRegistry 查询）
    private readonly Dictionary<string, ulong> _fullNameToTypeId = new();

    // GUID → ScriptTypeId（组合查询缓存）
    private readonly Dictionary<Guid, ulong> _guidToTypeId = new();

    /// <summary>根据 GUID 查询 ScriptTypeId。</summary>
    public bool TryGetScriptTypeId(Guid assetGuid, out ulong scriptTypeId);

    /// <summary>根据 GUID 查询 FullName。</summary>
    public bool TryGetFullName(Guid assetGuid, out string fullName);

    /// <summary>注册/更新一个脚本资产映射。</summary>
    public void Register(Guid assetGuid, string fullName);

    /// <summary>移除一个脚本资产映射。</summary>
    public void Unregister(Guid assetGuid);

    /// <summary>全量重建索引（Editor 启动时调用）。</summary>
    public void RebuildAll();
}
```

### 4.2 双阶段初始化（解决 ScriptRegistry 未就绪问题）

```
Phase A（Editor 启动，ScriptRegistry 可能未就绪）
  → 只建 GUID → FullName 映射
  → TypeId 暂时为 0

Phase B（ScriptRegistry Ready / 编译完成）
  → FullName → ScriptRegistry.FindByFullName() → TypeId 补全
  → GUID → TypeId 缓存填充
```

**Index 永远先活着，TypeId 是后绑定字段，不卡 Editor 启动顺序。**

### 4.3 构建时机

| 时机 | 触发 | 操作 |
|------|------|------|
| Editor 启动 | `ScriptEditorFeature.Initialize()` | `RebuildAll()` — Phase A：扫描所有 .cs 资产，只建 GUID → FullName |
| ScriptRegistry Ready | 编译完成 / Assembly 加载 | `RefreshAllTypeIds()` — Phase B：FullName → TypeId 补全 |
| 脚本编译成功 | `ScriptCompileQueue` 回调 | `Register(guid, fullName)` + 查询 TypeId |
| 脚本删除 | 资产删除事件 | `Unregister(guid)` — 移除映射 |

### 4.3 RebuildAll 流程

```
1. 遍历 EditorAssetDatabase 中所有 typeId == CSharpScript (11) 的资产
2. 对每个 .cs 资产：
   a. 获取 OS 文件路径
   b. 读取源文件
   c. 解析 namespace + class → FullName（一次性，离线操作）
   d. Register(guid, fullName)
3. 查询 ScriptRegistry 刷新 TypeId 映射
```

### 4.4 FullName 解析策略（仅用于 Index 构建）

**在 ScriptAssetIndex.RebuildAll() 中一次性执行，不在拖拽时实时解析**。

```csharp
/// <summary>
/// 从 .cs 源文件解析脚本类的 FullName。
/// ⚠️ 仅用于 Index 构建（离线操作），不用于拖拽时实时解析。
/// </summary>
private static string? ParseScriptFullName(string sourceCode)
{
    // 提取 namespace（支持 file-scoped 和 block-scoped）
    string? ns = null;
    var nsMatch = Regex.Match(sourceCode, @"namespace\s+([\w.]+)");
    if (nsMatch.Success)
        ns = nsMatch.Groups[1].Value;

    // 提取第一个 class 名称
    var classMatch = Regex.Match(sourceCode, @"class\s+(\w+)");
    if (!classMatch.Success)
        return null;

    string className = classMatch.Groups[1].Value;
    return string.IsNullOrEmpty(ns) ? className : $"{ns}.{className}";
}
```

**边界情况处理**:
| 场景 | 处理 |
|------|------|
| partial class | 取第一个声明（FullName 相同） |
| 多层嵌套 namespace | Regex 匹配完整 namespace 路径 |
| 一个文件多个 class | 取第一个（与 EntityBehaviour 模板一致） |
| class 内嵌 class | 不匹配（只匹配顶层 class） |
| #if 条件编译 | 可能误匹配，Index 构建时可接受（非关键路径） |
| 解析失败 | 不注册到 Index，拖拽时显示 "Unknown Script" |

---

## 5. ScriptInspector 设计

### 5.1 类定义

```csharp
/// <summary>
/// 脚本组件 Inspector——编辑 NNScriptComponentData。
/// </summary>
public sealed class ScriptInspector : ComponentTypeInspector<NNScriptComponentData>
{
    // ComponentTypeId 自动从 [ComponentId(0x9565553D163FC92A)] 读取
    // DisplayName = "Script"
    // Order = 200
}
```

### 5.2 UI 状态机（3 种状态）

| 状态 | 条件 | 显示 |
|------|------|------|
| **None** | `ScriptTypeId == 0` | "Script: None" + "Drop .cs script here" |
| **Uncompiled** | `ScriptTypeId != 0` 且 `ScriptRegistry.FindByTypeId() == null` | "Script: Uncompiled (PlayerController)" + Enabled |
| **Bound** | `ScriptTypeId != 0` 且 `ScriptRegistry.FindByTypeId() != null` | "Script: PlayerController" + Enabled |

### 5.3 UI 渲染

```
┌─ Script ──────────────────────────────────┐
│ Script: [PlayerController]        [×]     │  ← 脚本短名（从 ScriptRegistry 反查）
│ Enabled: [✓]                              │  ← Checkbox
│                                           │
│ [  Drop .cs script here  ]               │  ← 拖拽目标区域
└───────────────────────────────────────────┘
```

### 5.3 DrawFields 逻辑

```csharp
protected override bool DrawFields(ref NNScriptComponentData data)
{
    bool modified = false;

    // 1. 脚本名称显示
    string scriptName = "None";
    if (data.ScriptTypeId != 0)
    {
        var scriptInfo = ScriptRegistry.FindByTypeId(data.ScriptTypeId);
        scriptName = scriptInfo?.Name ?? "Unknown Script";
    }
    ImGui.Text($"Script: {scriptName}");

    // 2. Enabled checkbox
    bool enabled = data.Enabled != 0;
    if (ImGui.Checkbox("Enabled", ref enabled))
    {
        data.Enabled = enabled ? (byte)1 : (byte)0;
        modified = true;
    }

    // 3. 拖拽目标区域
    ImGui.Separator();
    ImGui.TextColored(new Vector4(0.5f, 0.5f, 0.5f, 1f),
        data.ScriptTypeId == 0 ? "Drop .cs script here" : "Drop .cs to change script");

    using (AssetDragDrop.BeginDragDropTarget())
    {
        if (AssetDragDrop.TryAcceptDragDrop(AssetDragDrop.Script, out var guid, out _))
        {
            // 通过 ScriptAssetIndex 查询 ScriptTypeId
            if (ScriptAssetIndex.Instance.TryGetScriptTypeId(guid, out var scriptTypeId))
            {
                data.ScriptTypeId = scriptTypeId;
                data.Enabled = 1;
                modified = true;
            }
        }
    }

    // 4. 右键菜单清除
    if (ImGui.BeginPopupContextItem())
    {
        if (ImGui.MenuItem("Remove Script"))
        {
            data.ScriptTypeId = 0;
            data.Enabled = 1;
            modified = true;
        }
        ImGui.EndPopup();
    }

    return modified;
}
```

---

## 6. AssetDragDrop 扩展

### 6.1 新增常量

```csharp
// Payload 名称
public const string Script = "NN_SCRIPT";

// TypeId 常量
public const ulong TypeIdCSharpScript = 11;
```

### 6.2 映射更新

```csharp
public static string GetPayloadName(ulong typeId)
{
    return typeId switch
    {
        TypeIdTexture2D => Texture,
        TypeIdAudioClip => Audio,
        TypeIdVideoClip => Video,
        TypeIdMesh => Mesh,
        TypeIdMaterial => Material,
        TypeIdCSharpScript => Script,  // 新增
        _ => Any,
    };
}

public static string GetPayloadNameByImporter(string importerType)
{
    return importerType switch
    {
        "TextureImporter" => Texture,
        "AudioImporter" => Audio,
        "VideoImporter" => Video,
        "MeshImporter" => Mesh,
        "MaterialImporter" => Material,
        "ScriptAssetImporter" => Script,  // 新增
        _ => Any,
    };
}
```

### 6.3 TryAcceptAnyDragDrop 更新

```csharp
string[] payloadNames = [Texture, Audio, Video, Mesh, Material, Script, Any];  // 新增 Script
```

---

## 7. 修改清单

| 文件 | 操作 | 状态 |
|------|------|------|
| `Neverness.Editor.Framework/Public/AssetDragDrop.cs` | 修改 | [x] 已完成 |
| `Neverness.Editor.Script/Private/ScriptAssetIndex.cs` | **新建** | [x] 已完成 |
| `Neverness.Editor.Scene/Private/Inspector/EcsScriptInspector.cs` | **新建** | [x] 已完成 |
| `Neverness.Editor.Script/Private/ScriptEditorFeature.cs` | 修改 | [x] 已完成 |
| `Neverness.Editor.Scene/Neverness.Editor.Scene.csproj` | 修改 | [x] 已完成（添加 Editor.Script 引用） |

### 不需要修改的文件
- `DetailInspector.cs` — 自动发现机制
- `ComponentInspectorRegistry.cs` — 自动扫描
- `ContentBrowserPanel.cs` — 拖拽源已自动处理
- `ScriptAssetImporter.cs` — 导入逻辑不变
- `ScriptRegistry.cs` — 不动（已有 TypeId ↔ Type 映射）
- `ScriptBehaviourScheduler.cs` — 不动
- `BehaviourRegistry.cs` — 不动

---

## 8. 数据流

### 添加 Script 组件

```
用户点击 "+ Add Component" → "Script"
  → SceneNativeBridge.AddComponent<NNScriptComponentData>(scene, entity)
  → Native ECS: Emplace<NNScriptComponent>(entity, {ScriptTypeId=0, Enabled=1})
  → Inspector 自动发现 Script 组件，渲染 ScriptInspector
  → 显示 "Script: None" + "Drop .cs script here"
```

### 拖拽 .cs 资产绑定

```
ContentBrowser 拖拽 PlayerController.cs
  → AssetDragDrop.SetDragDropPayload(guid, "ScriptAssetImporter", "PlayerController")
  → payload = "NN_SCRIPT"（新映射）

Inspector 拖拽目标
  → ScriptInspector.DrawFields() 中:
    AssetDragDrop.TryAcceptDragDrop("NN_SCRIPT", out guid, out _)
    → ScriptAssetIndex.TryGetScriptTypeId(guid, out scriptTypeId)
      ├─ 成功 → data.ScriptTypeId = scriptTypeId（来自 ScriptRegistry）
      └─ 失败 → 显示 "Unknown Script"，不写入 ECS
    → SetComponent(scene, entity, data)
```

### ScriptTypeId 权威链

```
拖拽时：
  GUID → ScriptAssetIndex.FullName → ScriptRegistry.FindByFullName() → TypeId
                                         ↑ 唯一权威来源

Index 构建时：
  .cs 文件 → 源文件解析 FullName → ScriptAssetIndex.Register(guid, fullName)
  ScriptRegistry 注册类型时 → ScriptAssetIndex.RefreshTypeId(fullName)
```

---

## 9. 实施顺序

```
Step 1: AssetDragDrop 扩展（Script payload 类型）
Step 2: ScriptAssetIndex 实现（GUID ↔ FullName ↔ ScriptTypeId 映射）
Step 3: ScriptInspector 实现（UI + 拖拽）
Step 4: ScriptEditorFeature 注册（Inspector + Index 初始化）
Step 5: 验证
```

---

## 10. 验证清单

- [ ] "+ Add Component" 菜单中出现 "Script" 选项
- [ ] 点击后实体添加 NNScriptComponentData（ScriptTypeId=0, Enabled=1）
- [ ] Inspector 显示 Script 组件区域（Script: None, Enabled: ✓）
- [ ] ContentBrowser 拖拽 .cs 文件到 Inspector 的拖拽区域
- [ ] 拖拽后 ScriptTypeId 正确（从 ScriptRegistry 查询，非源文件解析）
- [ ] Inspector 显示脚本短名（如 "PlayerController"）
- [ ] Enabled checkbox 可切换
- [ ] 右键菜单可移除 Script 组件
- [ ] ScriptTypeId=0 时显示 "None (drop .cs script here)"
- [ ] 脚本未编译时拖拽显示 "Unknown Script"（ScriptRegistry 未注册）
- [ ] 编译成功后 Inspector 自动刷新为正确脚本名
- [ ] Scheduler 不反向修改 ECS（硬规则）
