# RmlUI HTML/CSS 外部修改热重载 — 实施计划

> 日期: 2026-06-25
> 状态: **已完成**

---

## 一、现状分析

### 已有的链路（C# 端）

```
.html/.css 文件变更
  → AssetWatcher 检测（200ms 防抖 + SHA256 过滤）
  → HotReloadCoordinator.ProcessEvent()
  → ImportPipeline.Reimport()（更新 .nnasset）
  → AssetHandle.MarkForReload()（标记 Runtime 重载）
  → EditorEventBus.AssetReloaded 事件
```

**这条链路对 `.html`/`.css` 已经生效**（`AssetWatcherFilter` 没有过滤这两种扩展名）。

### 缺失的环节

**问题: Native RmlUI 渲染器有缓存，不会自动重新加载**

C++ RmlUI 渲染器按 VFS 路径缓存了 parsed HTML/CSS。即使 C# 端 reimport 了资产，native 端不会重新读取文件。需要一个主动通知机制告诉 native 端"这个文档变了，请重新加载"。

---

## 二、方案设计：API 函数通知

### 方案选型

| 方案 | 优点 | 缺点 |
|------|------|------|
| `RmlDocumentEntry.Version` 字段 | 不需要新 API，帧同步 | 结构体变大，ABI 变动，每帧传冗余字段 |
| **新增 API 函数（采用）** | 不碰结构体，语义清晰，低频事件低开销 | 需要新增 Native API 入口，需处理并发 |

**选择 API 函数方案的理由：** 热重载是低频事件（用户改文件才触发），用帧同步的 version 字段处理低频事件属于过度设计。API 调用语义更符合"主动通知"的直觉。

### 并发处理

C++ 侧把重载请求入队，下一帧开始渲染前统一执行，和已有的渲染命令队列模式一致。不需要锁。

### CSS 级联

CSS 变更时调 `NNRmlui_ReloadAllDocuments()`。Editor 场景下文档数量少（个位数），全量刷新没有问题。

---

## 三、详细改动

### 改动 1: Native API 新增两个函数

**位置:** `NNViewportRenderApi` 结构体（`NNNativeEngineApiTypes.cs`）

```csharp
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNViewportRenderApi
{
    // ── 现有 ──
    public delegate* unmanaged<uint, uint, void> SetRmlUIViewportSize;
    public delegate* unmanaged<uint, int, int, int, int, uint, uint, uint, void> ProcessRmlUIInput;
    public delegate* unmanaged<ulong> GetLastRmluiTexture;

    // ── 新增：RmlUI 文档热重载 ──
    /// <summary>通知 native 端指定文档需要重新加载（UTF-8 VFS 路径）。</summary>
    public delegate* unmanaged<byte*, void> ReloadRmlDocument;
    /// <summary>通知 native 端所有文档需要重新加载。</summary>
    public delegate* unmanaged<void> ReloadAllRmlDocuments;
}
```

**C++ 端同步修改:**
```cpp
struct NNViewportRenderAPI
{
    // 现有...
    void (*ReloadRmlDocument)(const char* vfsPath);
    void (*ReloadAllRmlDocuments)();
};
```

### 改动 2: C++ 端热重载队列 + FlushRmlReloads

**位置:** `RmlUIRuntimeApi.cpp`

```cpp
// 热重载请求队列
std::vector<std::string> s_PendingReloadPaths;
bool s_ReloadAll = false;

void NN_ENGINE_ABI_STDCALL rt_rmlUI_reloadDocument(const char* vfsPath)
{
    if (vfsPath && vfsPath[0] != '\0')
        s_PendingReloadPaths.emplace_back(vfsPath);
}

void NN_ENGINE_ABI_STDCALL rt_rmlUI_reloadAllDocuments(void)
{
    s_ReloadAll = true;
    s_PendingReloadPaths.clear();
}

// 渲染帧开始前调用
void FlushRmlReloads()
{
    if (!g_RmlUIRenderer) return;
    if (s_ReloadAll)
    {
        g_RmlUIRenderer->ReloadAllDocuments();
        s_ReloadAll = false;
        s_PendingReloadPaths.clear();
        return;
    }
    for (const auto& path : s_PendingReloadPaths)
        g_RmlUIRenderer->ReloadDocumentByPath(path);
    s_PendingReloadPaths.clear();
}
```

### 改动 3: RmlUIRenderer 新增热重载方法

**位置:** `RmlUIRenderer.h` / `RmlUIRenderer.cpp`

```cpp
/// 按 VFS 路径关闭并重新加载指定文档
void ReloadDocumentByPath(const std::string& vfsPath);

/// 关闭并重新加载所有文档
void ReloadAllDocuments();
```

实现逻辑：
- `ReloadDocumentByPath`：遍历 `m_Documents`，查找 `SourceURL` 匹配的文档，`Close()` 后从 map 移除
- `ReloadAllDocuments`：关闭全部文档，清空 `m_Documents`
- 下次 `Sync()` 时三路 Diff 自动重新加载（路径仍在 DrawList 中）

### 改动 4: 渲染管线插入 FlushRmlReloads

**位置:** `ViewportSurfaceRuntimeApi.cpp` → `RenderViewportCommands`

```cpp
// 热重载：处理 C# 端入队的重载请求（在 Sync 之前）
FlushRmlReloads();
rmlRenderer->Sync(rmlDrawItems);
```

### 改动 5: C# RmlDocumentReloadService

**位置:** `Neverness.Editor.Rmlui` 模块（新增文件）

```csharp
public sealed class RmlDocumentReloadService
{
    public RmlDocumentReloadService(IEditorEventBus eventBus)
    {
        eventBus.Subscribe(EditorEventType.AssetReloaded, OnAssetReloaded);
    }

    private void OnAssetReloaded(EditorEvent evt)
    {
        if (evt.Payload is not AssetReloadedEventPayload payload) return;
        var ext = payload.Path.Extension.ToLowerInvariant();
        if (ext is ".html" or ".htm" or ".rml")
            RmluiNativeApi.ReloadDocument(payload.Path.FullPath);
        else if (ext is ".css" or ".rcss")
            RmluiNativeApi.ReloadAllDocuments();
    }
}
```

### 改动 6: RmluiModule 注册服务

**位置:** `RmluiModule.cs`

```csharp
s_reloadService = new RmlDocumentReloadService(EditorCoreModule.Context.Events);
```

---

## 四、完整链路

```
.html/.css 文件变更
  → AssetWatcher 检测（200ms 防抖 + SHA256）
  → HotReloadCoordinator.ProcessEvent()
  → ImportPipeline.Reimport() → AssetHandle.MarkForReload()
  → EditorEventBus.AssetReloaded 事件
  → RmlDocumentReloadService.OnAssetReloaded()
    → .html/.htm/.rml → RmluiNativeApi.ReloadDocument(path)
    → .css/.rcss      → RmluiNativeApi.ReloadAllDocuments()
  → C++ rt_rmlUI_reloadDocument() 入队
  → 下一帧 RenderViewportCommands:
    → FlushRmlReloads()
      → RmlUIRenderer::ReloadDocumentByPath()
        → doc->Close() + 从 m_Documents 移除
    → Sync() → 检测到文档不在 map → 重新加载
    → Update() → Render()
```

---

## 五、影响的文件清单

### C# 端

| 文件 | 模块 | 改动 |
|------|------|------|
| `NNNativeEngineApiTypes.cs` | Runtime.Engine | `NNViewportRenderApi` 增加 2 个函数指针 |
| `RmluiNativeApi.cs` | Runtime.Rmlui | **新增** — P/Invoke 包装（ReloadDocument / ReloadAllDocuments） |
| `RmlDocumentReloadService.cs` | Editor.Rmlui | **新增** — 订阅事件 + 调用 native API |
| `Module.cs` | Editor.Rmlui | 创建并注册 `RmlDocumentReloadService` |

### C++ 端

| 文件 | 改动 |
|------|------|
| `ViewportRenderAPI.h` | `NNViewportRenderAPI` 增加 2 个函数指针 |
| `ViewportRenderApiStubs.cpp` | 新增 2 个 stub 函数 |
| `RmlUIRuntimeApi.h` | +`FlushRmlReloads()` 导出声明 |
| `RmlUIRuntimeApi.cpp` | +队列/标志 + 2 个 API 函数 + `FlushRmlReloads()` + 接线 |
| `RmlUIRenderer.h` | +`ReloadDocumentByPath` / `ReloadAllDocuments` 声明 |
| `RmlUIRenderer.cpp` | +`ReloadDocumentByPath` / `ReloadAllDocuments` 实现 |
| `ViewportSurfaceRuntimeApi.cpp` | +`FlushRmlReloads()` 调用（Sync 之前） |
