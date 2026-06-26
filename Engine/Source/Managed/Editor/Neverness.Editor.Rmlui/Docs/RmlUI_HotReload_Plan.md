# RmlUI HTML/CSS 外部修改热重载 — 实施计划

> 日期: 2026-06-25
> 状态: 待审查

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
struct NNViewportRenderApi
{
    // 现有...
    void (*ReloadRmlDocument)(const char* vfsPath);
    void (*ReloadAllRmlDocuments)();
};
```

### 改动 2: 新增 `RmlDocumentReloadService`

**位置:** `Neverness.Editor.Rmlui` 模块（新增文件）

**职责:**
- 订阅 `EditorEventBus.AssetReloaded` 事件
- `.html`/`.htm`/`.rml` 变更 → 调用 `ReloadRmlDocument(path)`
- `.css`/`.rcss` 变更 → 调用 `ReloadAllRmlDocuments()`
- 提供 P/Invoke 包装方法

```csharp
using System.Runtime.InteropServices;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Rmlui;

/// <summary>
/// RmlUI 文档热重载服务。
///
/// 订阅 AssetReloaded 事件，当 .html/.css 文件变更时
/// 通知 native RmlUI 渲染器重新加载文档。
///
/// 使用方法：
///   var service = new RmlDocumentReloadService(eventBus);
///   // 自动订阅事件，无需手动调用
///
/// @threadsafe Reload 调用从主线程 Tick 触发（通过 HotReloadCoordinator 事件队列）。
/// </summary>
public sealed class RmlDocumentReloadService
{
    /// <summary>创建热重载服务并订阅事件。</summary>
    public RmlDocumentReloadService(IEditorEventBus eventBus)
    {
        eventBus.Subscribe(EditorEventType.AssetReloaded, OnAssetReloaded);
    }

    private void OnAssetReloaded(EditorEvent evt)
    {
        if (evt.Payload is not AssetReloadedEventPayload payload)
            return;

        var ext = payload.Path.Extension.ToLowerInvariant();

        if (ext is ".html" or ".htm" or ".rml")
        {
            /* HTML 自身变更：重载指定文档 */
            ReloadDocument(payload.Path.FullPath);
        }
        else if (ext is ".css" or ".rcss")
        {
            /* CSS 变更：全量重载（Editor 场景文档数量少） */
            ReloadAllDocuments();
        }
    }

    /* ======================== Native API 调用 ======================== */

    /// <summary>通知 native 端重新加载指定文档。</summary>
    private static unsafe void ReloadDocument(string vfsPath)
    {
        var api = GetRenderApi();
        if (api == null || api->ReloadRmlDocument == null)
            return;

        var pathBytes = System.Text.Encoding.UTF8.GetBytes(vfsPath + '\0');
        fixed (byte* p = pathBytes)
        {
            api->ReloadRmlDocument(p);
        }

        Console.WriteLine($"[RmlDocumentReloadService] 重载文档: {vfsPath}");
    }

    /// <summary>通知 native 端重新加载所有文档。</summary>
    private static unsafe void ReloadAllDocuments()
    {
        var api = GetRenderApi();
        if (api == null || api->ReloadAllRmlDocuments == null)
            return;

        api->ReloadAllRmlDocuments();
        Console.WriteLine("[RmlDocumentReloadService] 全量重载所有文档");
    }

    /// <summary>获取 NNViewportRenderApi 指针。</summary>
    private static unsafe NNViewportRenderApi* GetRenderApi()
    {
        // 通过已有的 native API 获取方式取得指针
        // 具体实现取决于项目的 native API 访问模式
        return NNRuntimeEngineServices.GetViewportRenderApi();
    }
}
```

### 改动 3: 注册服务

**位置:** `RmluiModule.cs` 或 `EditorApplicationRunner.cs`

```csharp
// 在 RmluiModule.Install() 中
var reloadService = new RmlDocumentReloadService(EditorCoreModule.Context.Events);
// 如需外部访问，注册到服务定位器
EditorCoreModule.Context.RegisterService(reloadService);
```

### 改动 4: C++ 端实现

**职责:**
- `ReloadRmlDocument(path)` → 将路径加入 `_pendingReloadPaths` 队列
- `ReloadAllRmlDocuments()` → 设置 `_reloadAll = true` 标志
- 渲染帧开始前 → 检查队列/标志，清除对应文档缓存，重新加载

```cpp
// 伪代码
static std::vector<std::string> s_pendingReloadPaths;
static bool s_reloadAll = false;

void NNRmlui_ReloadDocument(const char* vfsPath)
{
    s_pendingReloadPaths.push_back(vfsPath);
}

void NNRmlui_ReloadAllDocuments()
{
    s_reloadAll = true;
}

// 渲染帧开始前调用
void FlushRmlReloads()
{
    if (s_reloadAll)
    {
        RmlContext->UnloadAllDocuments();
        s_reloadAll = false;
        s_pendingReloadPaths.clear();
        return;
    }

    for (auto& path : s_pendingReloadPaths)
    {
        RmlContext->UnloadDocument(path);
    }
    s_pendingReloadPaths.clear();
}
```

---

## 四、影响的文件清单

### C# 端

| 文件 | 模块 | 改动 |
|------|------|------|
| `NNNativeEngineApiTypes.cs` | Runtime.Engine | `NNViewportRenderApi` 增加 2 个函数指针 |
| `RmlDocumentReloadService.cs` | Editor.Rmlui | **新增** — 订阅事件 + 调用 native API |
| `RmluiModule.cs` | Editor.Rmlui | 创建并注册 `RmlDocumentReloadService` |

### C++ 端

| 文件 | 改动 |
|------|------|
| `NNViewportRenderApi` 结构体 | 增加 2 个函数指针 |
| 函数实现文件 | 实现 `ReloadRmlDocument` / `ReloadAllRmlDocuments` |
| 渲染帧入口 | 调用 `FlushRmlReloads()` |

**注意:** C# 端的 `RmlDocumentEntry` 结构体**不需要修改**，ABI 不变。

---

## 五、需要确认的问题

1. **Native API 访问方式**：`NNViewportRenderApi*` 的获取方式是什么？是通过 `NNRuntimeEngineServices.GetViewportRenderApi()` 还是其他途径？

2. **C++ 端文件位置**：`ReloadRmlDocument` / `ReloadAllRmlDocuments` 的实现应该放在哪个 C++ 文件？`FlushRmlReloads()` 应该在渲染管线的哪个阶段调用？

3. **`NNViewportRenderApi` 结构体布局**：增加 2 个函数指针后，结构体大小从 24 bytes 变为 40 bytes。已有的代码是否通过 `sizeof` 或偏移量访问？是否需要保持向后兼容？

4. **RmlContext 的文档管理 API**：C++ 端的 RmlUI 集成是否暴露了 `UnloadDocument(path)` 或类似 API？还是需要自己实现缓存管理？
