# RmlUI ECS 渲染管线修复记录

> 日期：2026-05-31
> 模块：NNRuntimeRmlui / NNRuntimeEngineServices / NNRuntimeScene / Neverness.Editor.Assets
> 状态：已修复，RmlUI 在 Editor Scene View 中正常显示

---

## 问题概述

实现 RmlUI ECS 系统后，Scene View 中无法显示 RmlUI 内容。排查过程中发现了 **6 个独立问题**，涉及 ABI 布局、资产注册表、GL 状态管理、渲染架构等层面。

---

## 问题 1：C# ABI 布局不匹配

### 症状
Editor 运行时崩溃或数据错乱。

### 根因
C# `NNViewportRenderApi` 结构体只有 3 个函数指针，Native 有 5 个。由于 `NNViewportRenderApi` 嵌入在 `NNNativeEngineApi` 聚合体中（`StructLayout.Sequential`），偏移量错位导致后续所有子表字段读取错误。

同时 C# `NNRmlUIDocumentComponentData` 为 28 字节，C++ `static_assert` 验证为 32 字节（含 4B 尾部对齐填充），数据传输时 C# 端截断 4 字节。

### 修复
- `NNNativeEngineApiTypes.cs`：`NNViewportRenderApi` 补齐 `SetRmlUIViewportSize` + `ProcessRmlUIInput`（5→6 个函数指针，新增 `GetLastRmluiTexture`）
- `NNNativeEngineApiTypes.cs`：`NNRmlUIDocumentComponentData` 补 `uint _padding0`（28→32B）
- `NNRmlUIDocumentComponent.h`：注释 28→32 字节（与 `static_assert` 一致）

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | 补齐函数指针 + padding |
| `Engine/Source/Runtime/NNRuntimeScene/Include/Components/NNRmlUIDocumentComponent.h` | 修正注释 |

---

## 问题 2：IAssetResolver 未注入

### 症状
`RmlUIRenderer::LoadDocument()` 始终返回 `nullptr`，日志输出 `m_AssetResolver is null`。

### 根因
`ViewportRenderRuntimeApi.cpp` 的 `EnsureSceneRenderer()` 创建了 `g_RmlUIRenderer` 但从未调用 `SetAssetResolver()`。`m_AssetResolver` 恒为 `nullptr`，`LoadDocument()` 直接返回失败。

### 修复
新增 `AssetRegistryResolver` 类实现 `IAssetResolver` 接口，在 `EnsureSceneRenderer()` 中创建并注入：

```cpp
class AssetRegistryResolver final : public NN::Runtime::Scene::IAssetResolver
{
public:
    bool Resolve(NNGuid guid, char* outPath, std::uint32_t outPathSize) noexcept override
    {
        const auto* api = NNNativeEngineApi_GetRuntimeTable();
        if (!api || !api->assetRegistry.resolvePathByGuid)
            return false;
        int result = api->assetRegistry.resolvePathByGuid(guid, outPath,
            static_cast<std::size_t>(outPathSize));
        return result > 0;
    }
};
```

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | 新增 `AssetRegistryResolver` + 注入 `SetAssetResolver()` |

---

## 问题 3：资产注册表用错（旧表 vs 新表）

### 症状
`AssetResolver` 返回空路径，日志输出 `AssetResolver failed for GUID`。

### 根因
项目中存在两个独立的资产注册表：

| 注册表 | 模块 | GUID 前缀 | 访问方式 |
|--------|------|-----------|----------|
| `AssetRegistrySubsystem` | NNRuntimeEngine | `0x56474153` ('VGAS') | `NNEngineRuntime::Instance().AssetRegistry()` |
| `NNAssetRegistry` | NNAssetRegistry | `0x4E4E4153` ('NNAS') | `NNNativeEngineApi_GetRuntimeTable()->assetRegistry` |

初版 `AssetRegistryResolver` 调用了旧注册表 `AssetRegistrySubsystem`，但资产注册在新注册表 `NNAssetRegistry` 中。且返回值判断 `!= 0` 把失败的 `-1` 误判为 `true`。

### 修复
改用 `NNNativeEngineApi_GetRuntimeTable()->assetRegistry.resolvePathByGuid`，返回值改为 `> 0`。

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | `AssetRegistryResolver` 改用新注册表 API |

---

## 问题 4：NNAssetRegistry 启动时为空

### 症状
即使修复了注册表选择，`注册表资产数: 0`，所有 GUID 查找失败。

### 根因
编辑器启动流程：
1. `EditorAssetDatabase.Initialize()` → `TryLoadCache()` 从磁盘加载缓存到 C# 字典
2. C# 字典包含所有 path↔GUID 映射
3. **但从未同步到 Native NNAssetRegistry**

`EditorAssetDatabase.Register()` 会调用 `AssetDatabase.Register()` → `TryRegisterNative()` → native `RegisterAsset`，但 `Register()` 仅在新资产导入时调用，不在启动时调用。启动时只加载缓存到 C# 字典。

### 修复
在 `EditorAssetDatabase.Initialize()` 中，`TryLoadCache()` 之后调用新增的 `SyncCacheToNative()` 方法：

```csharp
private static void SyncCacheToNative()
{
    var count = 0;
    foreach (var kvp in s_pathToGuid)
    {
        if (AssetDatabase.Register(kvp.Key, kvp.Value))
            count++;
    }
    Console.WriteLine($"[EditorAssetDatabase] SyncCacheToNative: 同步 {count}/{s_pathToGuid.Count} 个资产到 NNAssetRegistry");
}
```

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Managed/Editor/Neverness.Editor.Assets/Public/EditorAssetDatabase.cs` | 新增 `SyncCacheToNative()` + 在 `Initialize()` 中调用 |
| `Engine/Source/Managed/Runtime/Neverness.Runtime.Assets/AssetDatabase.cs` | 添加调试日志 |

---

## 问题 5：RmlUI EndFrame() blit 到屏幕破坏 ImGui GL 状态

### 症状
调用 `RmlUIRenderer::Render()` 后 ImGui 编辑器冻结或渲染异常。

### 根因
`RenderInterface_GL3::EndFrame()` 执行以下操作：
1. MSAA resolve（blit 内部 FBO → 后处理 FBO）
2. **绑定 framebuffer 0（屏幕）**
3. **渲染全屏四边形到屏幕**（覆盖 ImGui 输出）
4. 恢复 GL state

在 Editor 中，`RenderSceneToTexture` 从 ImGui `OnGUI()` 内部调用。此时 ImGui 已在渲染帧中，`EndFrame()` 的全屏四边形渲染覆盖了 ImGui 的 framebuffer 0 输出。

### 修复
1. `RenderInterface_GL3` 新增 `EndFrameNoBlit()` 方法——执行 MSAA resolve + GL state 恢复，但跳过 blit 到屏幕
2. `RmlUIRenderer` 新增 `RenderToTexture()` 方法——使用 `EndFrameNoBlit()`，返回内部 FBO 纹理 ID
3. `RmlUIRenderer::RenderToTexture()` 中保存/恢复 ImGui 依赖的 GL 状态（program、VAO、VBO、FBO、texture binding）

```cpp
// 保存 ImGui 的 GL 状态
GLint prevProgram = 0, prevVAO = 0, prevVBO = 0, prevFBO = 0;
GLint prevTexture = 0, prevActiveTex = 0;
glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVBO);
glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTex);

// RmlUI 渲染到内部 FBO
m_Context->Update();
m_RenderInterface->BeginFrame();
m_Context->Render();
m_RenderInterface->EndFrameNoBlit();

// 恢复 ImGui 的 GL 状态
glUseProgram(prevProgram);
glBindVertexArray(prevVAO);
glBindBuffer(GL_ARRAY_BUFFER, prevVBO);
glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
glActiveTexture(prevActiveTex);
glBindTexture(GL_TEXTURE_2D, prevTexture);
```

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Runtime/NNRuntimeRmlui/Include/Rml/RmlUi_Renderer_GL3.h` | 新增 `EndFrameNoBlit()` 声明 |
| `Engine/Source/Runtime/NNRuntimeRmlui/Source/Rml/RmlUi_Renderer_GL3.cpp` | 实现 `EndFrameNoBlit()` |
| `Engine/Source/Runtime/NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h` | 新增 `RenderToTexture()` 声明 |
| `Engine/Source/Runtime/NNRuntimeRmlui/Source/Renderer/RmlUIRenderer.cpp` | 实现 `RenderToTexture()` + GL state 保存/恢复 |

---

## 问题 6：RmlUI 纹理替换而非叠加

### 症状
RmlUI 渲染后 Sprite 场景消失，只显示 RmlUI 内容。

### 根因
`RenderSceneToTexture` 返回的纹理 ID 被 RmlUI 纹理替换，而不是叠加显示。

### 修复
采用双纹理叠加架构：
1. `RenderSceneToTexture` 正常返回 Sprite 纹理 ID
2. 新增 `GetLastRmluiTexture()` API 返回 RmlUI 纹理 ID
3. C# `EditorViewport` 先用 `ImGui.Image()` 显示 Sprite，再用 `DrawList.AddImage()` 叠加 RmlUI

```csharp
// 1. Sprite 场景（底层）
ImGui.Image(spriteTextureId, viewportSize, uv0, uv1);

// 2. RmlUI 叠加层（顶层）
var drawList = ImGui.GetWindowDrawList();
drawList.AddImage(rmluiTextureId, cursorPos, cursorPos + viewportSize, uv0, uv1);
```

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Runtime/NNNativeEngineAPI/Include/ViewportRenderAPI.h` | 新增 `GetLastRmluiTexture` 函数指针 |
| `Engine/Source/Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | 新增 `rt_viewportRender_getLastRmluiTexture` + API 表注册 |
| `Engine/Source/Runtime/NNRuntimeNativeEngineAPIStub/Private/ViewportRender/ViewportRenderApiStubs.cpp` | 新增 stub |
| `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | `NNViewportRenderApi` 新增 `GetLastRmluiTexture` |
| `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/Panel/EditorViewport.cs` | 双纹理叠加显示 |

---

## 附加修复：LoadDocument 空路径防护

### 症状
`AssetResolver` 返回空路径时 `m_Context->LoadDocument("")` 导致 RmlUI 崩溃。

### 修复
`RmlUIRenderer::LoadDocument()` 增加空路径检查：

```cpp
if (path[0] == '\0')
{
    std::cerr << "[RmlUIRenderer] AssetResolver returned empty path for GUID (...)" << std::endl;
    return nullptr;
}
```

### 文件
| 文件 | 改动 |
|------|------|
| `Engine/Source/Runtime/NNRuntimeRmlui/Source/Renderer/RmlUIRenderer.cpp` | `LoadDocument()` 增加空路径防护 |

---

## 最终架构

```
EditorViewport.OnGUI()
  │
  ├─ RenderSceneToTexture(sceneHandle, width, height)
  │    ├─ SceneRenderer::Render()           → spriteTextureId
  │    ├─ NNRmlUISystem::Tick()             → DrawList
  │    └─ RmlUIRenderer::RenderToTexture()  → rmluiTextureId
  │         ├─ 保存 ImGui GL state
  │         ├─ Context::Update()
  │         ├─ BeginFrame() → 创建内部 FBO
  │         ├─ Context::Render() → 渲染到 FBO
  │         ├─ EndFrameNoBlit() → MSAA resolve + 恢复 GL state（不 blit 到屏幕）
  │         ├─ GetRenderResult().color_tex_buffer → 纹理 ID
  │         └─ 恢复 ImGui GL state
  │    └─ 返回 spriteTextureId
  │
  ├─ GetLastRmluiTexture() → rmluiTextureId
  │
  ├─ ImGui.Image(spriteTextureId)         ← 底层 Sprite 场景
  └─ DrawList.AddImage(rmluiTextureId)    ← 顶层 RmlUI 叠加层
```

---

## 资产注册链路

```
Editor 启动
  ├─ EditorAssetDatabase.Initialize()
  │    ├─ TryLoadCache()         ← 从磁盘加载 path↔GUID 到 C# 字典
  │    └─ SyncCacheToNative()    ← 将所有资产注册到 NNAssetRegistry ✅ (本次新增)
  │
  ├─ 场景加载
  │    └─ RmlUIDocument 组件引用 GUID
  │
  └─ RenderSceneToTexture
       └─ RmlUIRenderer::Sync()
            └─ LoadDocument(guid)
                 └─ AssetRegistryResolver::Resolve(guid)
                      └─ NNAssetRegistry::ResolvePathByGuid(guid) → "/assets/New Document.html"
                           └─ Context::LoadDocument("/assets/New Document.html") ✅
```

---

## 教训总结

1. **ABI 布局必须 C#/C++ 同步**：改 C++ struct 必须同步改 C# 端，否则聚合体偏移错位导致全局崩溃
2. **注册表不等于缓存**：C# 字典缓存和 Native 注册表是独立的，启动时必须同步
3. **GL state 必须完整保存/恢复**：子渲染器（RmlUI）修改的 GL state 可能超出父渲染器（ImGui）的备份范围，需要手动保存/恢复 program、VAO、VBO、FBO、texture binding
4. **EndFrame 不等于安全退出**：RmlUI 的 `EndFrame()` 会 blit 到 framebuffer 0，在子渲染器场景中必须使用 `EndFrameNoBlit()` 变体
5. **渲染结果需要合成而非替换**：多渲染器场景下，各渲染器输出应作为独立纹理叠加显示，而非替换返回值
