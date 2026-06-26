# 资产监视过滤 — 修改建议

> 日期: 2026-06-25
> 状态: **已实施**（方案 A，仅 .scene + .prefab）

---

## 问题

当前 `AssetWatcher` 监视 `Assets/` 下所有文件（除 `.meta`、dotfiles、`Library/`、`Temp/`）。但部分资产只在 Editor 内部修改，监视它们会导致 **Editor 自己保存 → 触发 Watcher → 重新导入** 的无意义循环。

---

## 资产类型分析

| 扩展名 | Importer | 修改来源 | 需要监视？ | 理由 |
|--------|----------|---------|-----------|------|
| `.scene` | SceneImporter | Editor 内部 | ❌ 不需要 | 场景保存由 EditorSceneManager 驱动，已走完整序列化流程 |
| `.prefab` | PrefabImporter | Editor 内部 | ❌ 不需要 | Prefab 保存由 Editor 驱动 |
| `.material` | MaterialImporter | Editor 内部 + 外部 | ✅ 需要 | 美术可能用外部文本编辑器批量修改材质参数 |
| `.png/.jpg/.jpeg/.tga/.bmp/.dds/.hdr` | TextureImporter | 外部工具 | ✅ 需要 | Photoshop、Krita 等外部修改 |
| `.fbx/.obj/.gltf/.glb` | MeshImporter | 外部工具 | ✅ 需要 | Blender、Maya 等外部修改 |
| `.lua` | LuaScriptImporter | 外部编辑器 | ✅ 需要 | VS Code、Sublime 等外部修改 |
| `.cs` | ScriptEditorServiceImpl | 外部 IDE | ✅ 需要 | 已有独立 Watcher，不在 AssetWatcher 范围 |
| `.shader` | （未实现） | — | — | 暂无 Importer |
| `.animation` | （未实现） | — | — | 暂无 Importer |
| `.html` | （未实现） | — | — | 暂无 Importer |

---

## 建议方案

### 方案 A: 扩展名白名单（推荐）

在 `AssetWatcher` 中维护一个 **不监视的扩展名集合**，这些扩展名的文件变更事件直接忽略。

**优点:** 简单明确，一个 HashSet 解决
**缺点:** 用户手动编辑 `.scene` JSON 时不触发 reimport（极小概率场景）

```csharp
/// <summary>
/// Editor 内部管理的资产扩展名——不需要 FileSystemWatcher 监视。
/// 这些资产的修改由 Editor 自身驱动，走 Editor 内部保存流程，
/// 不需要通过 Watcher → ImportPipeline 重新导入。
///
/// 注意：.material 不在此列，因为美术可能用外部文本编辑器批量修改材质参数。
/// </summary>
private static readonly HashSet<string> s_editorManagedExtensions = new(StringComparer.OrdinalIgnoreCase)
{
    ".scene",
    ".prefab",
};
```

在 `ShouldIgnore()` 中添加检查：

```csharp
private static bool ShouldIgnore(string path)
{
    /* 忽略 .meta 檔案 */
    if (path.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
        return true;

    /* 忽略暂存檔案 */
    var fileName = Path.GetFileName(path);
    if (fileName.StartsWith('.') || fileName.StartsWith('~'))
        return true;

    /* 忽略 Editor 内部管理的资产 */
    var ext = Path.GetExtension(path);
    if (!string.IsNullOrEmpty(ext) && s_editorManagedExtensions.Contains(ext))
        return true;

    return false;
}
```

### 方案 B: 可配置扩展名集合

将过滤集合暴露为 `AssetWatcher` 构造参数，由 `AssetsModuleImp` 注入。

**优点:** 更灵活，可在运行时调整
**缺点:** 复杂度更高，当前场景不需要

### 方案 C: 标记 Editor 正在写入的文件

Editor 保存时先将路径加入 `HashSet`，Watcher 检查到变更时跳过。

**优点:** 精确到文件级别，不影响同扩展名的外部修改
**缺点:** 需要在所有 Editor 保存路径上加标记，侵入性强

---

## 建议

**采用方案 A**。理由：
1. `.scene` / `.prefab` / `.material` 几乎不可能被用户手动编辑 JSON（没有文档格式）
2. 如果真需要手动编辑，用户可以临时关闭 Watcher 或用 `ForceFullScan()`
3. 实现最简单，改动最小（~10 行）

---

## 需要确认的问题

1. `.material` 文件是否可能被外部工具修改？比如美术用文本编辑器调材质参数？如果是，应该保留监视。
2. `.scene` / `.prefab` 的 JSON 格式是否有文档？用户是否可能手动编辑？
3. 未来是否会有其他 Editor 内部管理的扩展名？如果是，方案 B 更合适。
