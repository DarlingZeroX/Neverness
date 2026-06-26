# CSS Style 资产工厂实施记录

**日期**：2026-06-26
**模块**：Neverness.Editor.Rmlui

---

## 1. 需求

在 ContentBrowser 右键菜单中支持创建 `.css` 样式文件资产，与现有的 RmlUI Document (.html) 并列。

## 2. 架构分析

项目资产创建系统基于 `IAssetFactory` 接口 + 反射自动发现：

```
IAssetFactory 实现
    ↓ (反射扫描 Neverness.Editor.* 程序集)
AssetFactoryRegistry.Instance.Factories
    ↓ (按 Category 分组)
AssetCreationMenuContributor → ContentBrowser 右键菜单 "Create {DisplayName}"
```

**关键点**：只需实现 `IAssetFactory`，菜单自动生成，无需手动注册。

## 3. 实施内容

### 3.1 新建文件

**`AssetFactories/CssAssetFactory.cs`**

```csharp
public sealed class CssAssetFactory : IAssetFactory
{
    public string DisplayName => "CSS Style";
    public string Category => "UI";
    public string Icon => "🎨";
    public string FileExtension => ".css";

    public NPath? CreateAsset(NPath directoryPath)
    {
        // 递增命名：New Style.css, New Style 1.css, ...
        // 从 EditorResourceCache.TemplateNames.RmlStyle 加载模板
        // 兜底硬编码最小 CSS
    }
}
```

- 模板来源：`/editor/template/asset/style.css`（VFS 路径）
- 兜底模板：最小 `.container` 布局

### 3.2 修改文件

**`AssetTypeRegistry.cs`** — Neverness.Runtime.Assets

- 新增常量：`public const ulong HtmlDocument = 0x0000000C;`
- 构造函数注册：`RegisterType("HtmlDocument", TypeId.HtmlDocument);`
- CSS 和 HTML 共用同一类型 ID（12）

**`MetaFileManager.cs`** — Neverness.Editor.Assets

- `InferImporterName` 新增映射：`".css" or ".rcss" => "RmlStyleImporter"`

### 3.3 无需改动的部分

| 组件 | 原因 |
|---|---|
| `AssetCreationMenuContributor` | 自动从 `AssetFactoryRegistry` 读取所有工厂 |
| `AssetMeta.InferAssetTypeId` | 已有 `.css` → 12 (HtmlDocument) 映射 |
| `AssetTypeActionsRegistry` | 在 `AssetsModuleImp.Install()` 中自动从工厂注册 |
| `RmlUIContextMenuContributor` | SceneBrowser 实体菜单，CSS 不需要 |
| `RmluiModule.Install()` | 工厂通过反射发现，无需手动注册 |

## 4. 菜单效果

ContentBrowser 右键菜单自动出现：

```
Create ─┬─ Scene
        ├─ Material
        ├─ Lua Script
        ├─ C# Script
        └─ UI ──┬─ Create RmlUI Document (.html)
                └─ Create CSS Style (.css)       ← 新增
```

## 5. 文件清单

```
Neverness.Editor.Rmlui/
  AssetFactories/
    RmlDocumentAssetFactory.cs   (已有)
    CssAssetFactory.cs           (新建)
  Public/
    Module.cs
    RmlUIContextMenuContributor.cs
```

## 6. 模板文件

物理路径：`Resource/Editor/template/asset/style.css`
VFS 路径：`/editor/template/asset/style.css`
模板常量：`EditorResourceCache.TemplateNames.RmlStyle` = `"style.css"`
