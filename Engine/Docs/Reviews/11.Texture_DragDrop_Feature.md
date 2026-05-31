# ContentBrowser 拖拽 Texture 到 SpriteRendererInspector

## 概述

实现 ContentBrowser 中纹理资产到 SpriteRendererInspector Texture 字段的拖拽赋值功能。

## 架构

两个面板无编译时依赖，契约由 ImGui drag-drop 定义：
- Payload type: `"TEXTURE_ASSET"`
- Payload data: `GUID` (2 × ulong = 16 bytes)

Drop 解析链：
```
ContentBrowser 拖拽纹理文件
  → SetDragDropPayload("TEXTURE_ASSET", guid)
  → SpriteRendererInspector AcceptDragDropPayload
  → AssetHandle.LoadSync(guid, 1)
  → TextureInterop.LoadTextureFromAsset(handle)
  → data.TextureAsset = gpuKey
```

## 修改文件清单

| # | 文件 | 操作 | 说明 |
|---|------|------|------|
| 1 | `Editor/.../Assets/Private/Panel/ContentBrowserPanel.cs` | 修改 | 文件循环中添加纹理资产拖拽源 |
| 2 | `Editor/.../Scene/Private/Inspector/SpriteRendererInspector.cs` | 修改 | 纹理区域改为 drop zone + 右键清除 |

## 详细变更

### 1. ContentBrowserPanel 添加拖拽源

在 `DrawContentBrowser()` 的文件循环中，`DrawItemFunction` 之后、`ImGui.PushID` 之前插入：

- 判断条件：`item is ContentFile file && file.AssetType == "TextureImporter"`
- 通过 `EditorAssetDatabase.TryGetGuid(file.Path, out guid)` 获取资产 GUID
- `BeginDragDropSource` / `SetDragDropPayload("TEXTURE_ASSET", guid)` / `EndDragDropSource`
- Payload 打包为 `stackalloc ulong[2]`（GUID.High, GUID.Low）

### 2. SpriteRendererInspector 添加 drop zone

原 Texture 字段从只读显示改为完整的拖放交互区：

**Drop Zone**：`InvisibleButton("##TextureDropZone")` 作为可拖放区域（128px 高），用 DrawList 绘制：
- 有纹理时：预览缩略图（`AddImage`）或 "未加载" 文本
- 无纹理时：灰色 "None (drop texture here)" 提示

**Drop 接收**：`BeginDragDropTarget` / `AcceptDragDropPayload("TEXTURE_ASSET")`：
- 从 payload 解包 GUID（两个 ulong）
- `AssetHandle.LoadSync(guid, 1)` 同步加载资产
- `TextureInterop.LoadTextureFromAsset(handle.Value)` 获取 GPU 纹理 key
- 设置 `data.TextureAsset = gpuKey`

**右键清除**：`BeginPopupContextItem` 提供 "Clear Texture" 菜单项，将 `TextureAsset` 置零。

## 设计决策

1. **Payload = GUID**：16 字节固定大小，最通用，接收端可自主决定加载策略
2. **InvisibleButton 作为 drop zone**：`BeginDragDropTarget` 需要上一个 item 有区域，同时提供 hover 反馈
3. **同步加载**：`AssetHandle.LoadSync` 对纹理资产可接受，未来可改异步
4. **右键清除**：`BeginPopupContextItem` 不污染 UI 布局，ImGui 惯用模式
5. **无新抽象**：不引入 IDragDropService，ImGui 原生 drag-drop 已足够（与 SceneBrowser ENTITY 拖拽一致）
6. **`AssetTypeId.Texture2D` 使用字面量 `1`**：避免 `Editor.Scene` 引用 `Editor.Assets`，值与 Native NNAssetTypes.h 一致

## 后续关注

- **扩展其他资产类型**：同一模式可扩展到 Material / Mesh / Audio
- **异步加载**：大纹理可改为异步加载 + 占位符显示
- **拖拽视觉反馈**：可在 drop zone 绘制虚线边框

---

*文档版本：v1.0*
*最后更新：2026/05/27*
