# Runtime 渲染迁移计划：OpenGL → Diligent Engine（v3）

> **状态**: Phase 0-5 完成，Phase 6 实施中（待验证崩溃修复）
> **创建日期**: 2026-06-09
> **修订**: v6（Phase 6 实施进展）
> **目标**: 用 Experiments 的 Diligent 渲染替代 Runtime 的 OpenGL 渲染
> **约束**: ThirdParty 不修改，Experiments 可按需修改

---

## 施工进度

| Phase | 内容 | 状态 | 完成日期 |
|-------|------|------|---------|
| **Phase 0** | 构建系统（CMake 重排、C++17 对齐、PLATFORM_WIN32 PUBLIC） | ✅ 完成 | 2026-06-09 |
| **Phase 1** | RenderAssets（IRenderResourceFactory 工厂模式） | ✅ 完成 | 2026-06-09 |
| **Phase 2** | Renderer2D（NNRuntimeRender 接口重写、HLSL 着色器） | ✅ 完成 | 2026-06-09 |
| **Phase 3** | RmlUI（RmlDiligent 直接接入） | ✅ 完成 | 2026-06-10 |
| **Phase 4** | ImGui（Diligent 后端切换） | ✅ 完成 | 2026-06-10 |
| **Phase 5** | Application + SwapChain + RenderAssetManager 初始化 | ✅ 完成 | 2026-06-10 |
| Phase 6 | ViewportRender（C# Editor 纹理句柄） | 🔧 进行中 | — |
| Phase 7 | Legacy | ❌ 最后处理 | — |
| Phase 8 | RHI Deprecated | ❌ 最后处理 | — |

**真实施工顺序**：
```
Phase 0 ✅ → Phase 1 ✅ → Phase 2 ✅
→ Phase 3 ✅ → Phase 4 ✅ → Phase 5 ✅
→ Phase 6 ViewportRender
→ Phase 7 Legacy → Phase 8 Cleanup
```

**约束**：
1. 不创建 DiligentBridge
2. 不修改 ThirdParty（Experiments 可按需修改）
3. Runtime 直接链接 NNRuntimeRender + NNRuntimeDiligent + NNRuntimeRenderBootstrap
4. Renderer2D 着色器从 GLSL 转为 HLSL（NNRuntimeRender 的 CreateShader 只支持 HLSL）
5. SceneRenderer 返回 `reinterpret_cast<uint64_t>(ITextureView*)`
6. 纹理句柄通过 NNRenderAssetManager::GetImGuiTextureHandle() 获取

---

## 0. 决策摘要

| 决策项 | 选择 | 理由 |
|--------|------|------|
| 接口层 | 直接用 NNRuntimeRender | 废弃旧 VGFX，无中间层 |
| RmlUI | 直接复用 RmlDiligent | 已成熟，不包装不重写 |
| Renderer2D | 保留 GLSL，只迁移 API | Diligent 支持 GLSL 编译 |
| NNEngineLegacy | **完全冻结** | 不删 opengl32，不脱钩，不动任何东西 |
| NNRuntimeRHI | Deprecated，不立刻删 | 至少一个版本后再清理 |
| 纹理句柄 | `reinterpret_cast<uint64_t>(ITextureView*)` | Editor/Runtime 同进程，无需注册表 |
| DiligentBridge | **不做** | 多个 DLL 直接链接 STATIC 库即可 |
| Device 持有 | RenderResourceFactory | AssetManager 不管理 Device |
| ImGui 版本 | NNRuntimeImGui 版本优先 | Diligent 捆绑版本不一致，需要对齐 |
| Phase 顺序 | RenderAssets 先于 Application | Application 不是前提，ViewportRender 已有 Device |
| RenderAssetManager 初始化 | 延迟到设备创建后 | 设备不存在时工厂为空 |
| NNTextureDesc 命名冲突 | Runtime 版改名 NNTextureCacheDesc | 与 Experiments 版同名冲突 |
| 工厂模块归属 | NNRenderAssetManager 所在模块 | 工厂是接口实现，属于 RenderAssets |
| ImGui ini 文件 | 恢复 io.IniFilename | Diligent 后端构造函数置空了 |
| RenderPass 必须 Begin/End | PSO 关联 RenderPass 时必须 BeginRenderPass | 否则 GPU 挂死 |
| SRV vs RTV | ImGui 需要 SRV 采样 | RTV 不能用于着色器采样 |
| diliPSO/diliRenderPass 持有方式 | RefCntAutoPtr | 裸指针会导致悬空，堆损坏 |

---

## 1. 当前架构（OpenGL）

```
NNEngineLegacy (聚合器, 链接 opengl32)
  ├── NNRuntimeApplication → NNRuntimeRHI, NNRuntimeImGui
  ├── NNRuntimeRenderer2D  → NNRuntimeRHI, opengl32（直接调 GL）
  ├── NNRuntimeRmlUI       → NNRuntimeRHI, RenderInterface_GL3
  ├── NNRuntimeRenderAssets→ NNRuntimeRHI（OpenGL::Texture2D）
  ├── NNRuntimeImGui       → imgui_impl_opengl3, opengl32
  └── NNRuntimeRHI (SHARED)→ opengl32（VGFX 接口 + OpenGL 实现）

NNRuntimeEngineServices/ViewportRender
  └── 返回 OpenGL texture ID (uint32_t) 给 C# Editor
```

---

## 2. 目标架构（Diligent）

```
NNRuntimeRender (STATIC, 接口层)
NNRuntimeDiligent (STATIC, Diligent 实现)
NNRuntimeRenderBootstrap (STATIC, 设备工厂)
       ↑
       | 直接链接，无中间层
       |
Runtime 模块 (SHARED DLL)
  ├── NNRuntimeApplication
  ├── NNRuntimeImGui（ImGui 版本优先，Diligent 对齐）
  ├── NNRuntimeRenderAssets
  ├── NNRuntimeRenderer2D
  └── NNRuntimeRmlui → 直接链接 RmlDiligent

纹理句柄: reinterpret_cast<uint64_t>(ITextureView*)
  └── Editor/Runtime 同进程，直接传指针
  └── C# 侧 uint64_t → ImGui.Image()
  └── 无需注册表，无泄漏风险

NNEngineLegacy: 完全冻结，不删不改不动
NNRuntimeRHI: Deprecated，保留至少一个版本
```

---

## 3. Phase 0：构建系统 ✅

**目标**: Runtime 能链接 Experiments STATIC 库

### 3.1 CMake 构建顺序 ✅

**文件**: `Engine/Source/CMakeLists.txt`

```cmake
add_subdirectory("Core")
add_subdirectory("Experiments")  # ← 先于 Runtime，无条件编译
add_subdirectory("Runtime")
```

原 `NEVERNESS_BUILD_EXPERIMENTS` option 已移除。

### 3.2 C++ 标准对齐 ✅

全部统一为 C++17（包括 Experiments）：

| 模块 | 变更 |
|------|------|
| Experiments 主 CMakeLists.txt | C++20 → C++17 |
| NNRuntimeCore | C++20 → C++17 |
| NNRuntimeRender | C++20 → C++17 |
| NNRuntimeDiligent | C++20 → C++17 |
| NNRuntimeRenderAssets (Exp) | C++20 → C++17 |
| NNRuntimeSRP | C++20 → C++17 |
| NNRuntimeRenderBootstrap | C++20 → C++17 |
| NNRuntimeNativeEngineAPI | C++20 → C++17 |
| RmlDiligent | C++20 → C++17 |

Runtime 模块保持 C++17 不变。

### 3.3 平台定义传播 ✅

`NNRuntimeDiligent/CMakeLists.txt` 中 `PLATFORM_WIN32` 从 `PRIVATE` 改为 `PUBLIC`，自动传播给所有链接它的 target。

### 3.4 NVAPI stub 独立 ✅

NVAPI stub 从 `RmlDiligent/Stubs/` 移到 `ThirdParty/nvapi_stub/`，独立编译。`DILIGENT_NVAPI_PATH` 指向新位置。

### 3.5 不做 DiligentBridge ✅

Runtime 模块直接链接 Experiments STATIC 库。

### 3.6 验收标准

- [x] 全项目编译通过
- [x] Experiments 在 Runtime 之前构建
- [x] 全部 C++17
- [x] PLATFORM_WIN32 正确传播
- [x] NVAPI stub 独立编译

---

## 4. Phase 0.5：ImGui 版本验证

**目标**: 确认 NNRuntimeImGui 的 ImGui 版本与 Diligent 捆绑版本一致，不一致则对齐

**为什么需要**: NNRuntimeImGui 使用自己的 ImGui 版本，Diligent 捆绑了另一个版本。`imgui_internal.h`、`ImDrawList`、`ImGuiContext` 的结构在不同版本间不兼容，直接混用会炸。

### 4.1 版本检查清单

```
需要验证的库：
  ImGui（核心）
  ImNodeEditor
  ImGuizmo
  ImNodes
  ImGuiColorTextEdit

检查项：
  各库使用的 ImGui 版本号是否一致
  imgui.h 版本宏 IMGUI_VERSION
  imgui_internal.h 中 ImGuiContext 结构是否一致
```

### 4.2 对齐策略

**情况 A：版本一致** → 直接进入 Phase 1

**情况 B：版本不一致** → 以 NNRuntimeImGui 的版本为准：

```cmake
# NNRuntimeDiligent 停止编译 Diligent 捆绑的 ImGui
# 改为链接 NNRuntimeImGui 的 ImGui 核心
target_link_libraries(NNRuntimeDiligent PRIVATE NNRuntimeImGui)
target_include_directories(NNRuntimeDiligent PRIVATE
    ${NNRUNTIME_IMGUI_DIR}  # NNRuntimeImGui 的 ImGui 头文件
)
```

Diligent 的 ImGui 后端（`ImGuiImplDiligent`）通常只依赖 ImGui 公共 API，版本对齐后直接可用。

### 4.3 验收标准

- [ ] 所有 ImGui 扩展库版本号一致
- [ ] NNRuntimeImGui 和 NNRuntimeDiligent 使用同一份 ImGui 核心
- [ ] 编译无重复符号
- [ ] 运行时 ImGuiContext 正确共享

---

## 5. Phase 1：RenderAssets ✅

**目标**: 用 IRenderResourceFactory 替换 OpenGL 纹理创建

### 5.1 工厂模式 ✅

引入 `IRenderResourceFactory` 接口，AssetManager 通过工厂创建资源，不直接持有 Device。

### 5.2 CMake 变更 ✅

```cmake
# 移除
NevernessRuntime-RHI, opengl32

# 添加
NNRuntimeRender（接口层）
```

### 5.3 实际修改文件

| 文件 | 变更 |
|------|------|
| `Runtime/NNRuntimeRenderAssets/CMakeLists.txt` | 依赖替换，添加 Experiments include 路径 |
| `Runtime/NNRuntimeRenderAssets/Include/NNRenderAssetManager.h` | 新增 IRenderResourceFactory 接口 + m_Factory 成员 |
| `Runtime/NNRuntimeRenderAssets/Private/NNRenderAssetManager.cpp` | 移除 OpenGL 头文件和调用，改用工厂 |
| `Runtime/NNRuntimeRenderAssets/Include/NNTextureResource.h` | 更新注释 |
| `Runtime/NNRuntimeRenderAssets/Private/NNTextureResource.cpp` | 移除 OpenGL 依赖 |
| `Runtime/NNRuntimeEngineServices/Private/Registry/NativeEngineRuntimeApiTable.cpp` | Initialize(nullptr) 临时占位 |

### 5.4 验收标准

- [x] NNRenderAssetManager 无 OpenGL 头文件
- [x] 纹理创建通过 IRenderResourceFactory
- [x] AssetManager 不持有 Device
- [x] 编译通过

---

## 6. Phase 2：Renderer2D ✅

**目标**: 用 NNRuntimeRender 接口替换 OpenGL 调用

### 6.1 着色器：GLSL → HLSL ✅

NNRuntimeRender 的 `CreateShader` 实现目前只支持 HLSL（硬编码 `SHADER_SOURCE_LANGUAGE_HLSL`）。GLSL 需要转换为 HLSL，逻辑完全一致。

### 6.2 实际修改文件

| 文件 | 变更 |
|------|------|
| `Runtime/NNRuntimeRenderer2D/CMakeLists.txt` | 移除 RHI/opengl32，添加 NNRuntimeRender/NNRuntimeDiligent |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/BuiltinShaders.h` | GLSL → HLSL |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/Renderer2D.h` | Initialize() 添加 INNRenderDevice* 参数 |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/FramebufferObject.h` | 改用 INNRenderTarget，GetColorTextureHandle() |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/SceneRenderer.h` | Initialize(INNRenderDevice*)，Render 返回 uint64_t |
| `Runtime/NNRuntimeRenderer2D/Source/Renderer2D.cpp` | 完整重写：raw Diligent PSO + SRB + MapHelper |
| `Runtime/NNRuntimeRenderer2D/Source/FramebufferObject.cpp` | 改用 INNRenderTarget |
| `Runtime/NNRuntimeRenderer2D/Source/SceneRenderer.cpp` | 移除 OpenGL 调用 |
| `Runtime/NNRuntimeRenderer2D/Source/SpriteRenderSystem.cpp` | GetGLTextureId → GetImGuiTextureHandle |
| `Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | Initialize(nullptr) 临时占位 |

### 6.3 关键技术决策

- **PSO 创建**: 使用 raw Diligent `GraphicsPipelineStateCreateInfo`（NNRuntimeRender 的 `CreatePipelineState` 缺少 `DefaultVariableType` 控制）
- **SRB 缓存**: 每个纹理一个 SRB，缓存在 `unordered_map<void*, RefCntAutoPtr<IShaderResourceBinding>>`
- **CB 更新**: 使用 Diligent `MapHelper<SpriteCB>` 做 Map/Unmap
- **Blend 状态**: 固定 Alpha 混合（PSO 创建时设置，不支持运行时切换）

### 6.4 验收标准

- [x] Renderer2D 无 OpenGL 头文件/调用
- [x] 着色器为 HLSL
- [x] 通过 INNRenderDevice 创建资源
- [x] 通过 raw Diligent context 录制命令
- [x] FramebufferObject 使用 INNRenderTarget
- [x] 编译通过

---

## 7. Phase 3：RmlDiligent 接入 ✅

**目标**: 直接接入 RmlDiligent，不包装不重写

### 7.1 设计

直接链接 `rmlui_backend_diligent`（Experiments 的 STATIC 库），替换 RenderInterface_GL3。

```
NNRuntimeRmlui (SHARED)
  └── 链接 rmlui_backend_diligent (STATIC)
  └── 链接 NNRuntimeDiligent (STATIC)
```

### 7.2 CMake 变更

```cmake
# 移除
link_libraries(NevernessRuntime-RHI)

# 添加
link_libraries(rmlui_backend_diligent NNRuntimeDiligent)
```

### 7.3 RmlUIRenderer 改造

**文件**: `Runtime/NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h`

```cpp
// 移除
#include "Rml/RmlUi_Renderer_GL3.h"

// 添加
#include "RmlDiligentRenderInterface.h"

class RmlUIRenderer {
    std::unique_ptr<RmlDiligent::RmlDiligentRenderInterface> m_RenderInterface;
    uint64_t RenderToTexture(); // 原 uint32_t，返回 NNTextureHandle
};
```

### 7.4 移除旧文件

- `Runtime/NNRuntimeRmlui/Include/Rml/RmlUi_Renderer_GL3.h` — 删除
- `Runtime/NNRuntimeRmlui/Source/Rml/RmlUi_Renderer_GL3.cpp` — 删除

### 7.5 验收标准

- [ ] RmlUIRenderer 使用 RmlDiligentRenderInterface
- [ ] RenderToTexture() 返回 NNTextureHandle
- [ ] HTML/CSS 渲染视觉一致

---

## 8. Phase 4：ImGui 后端切换 ✅

**目标**: 用 Diligent 的 ImGui 后端替换 imgui_impl_opengl3，ImGui 版本以 NNRuntimeImGui 为准

**前提**: Phase 0.5 版本验证通过

### 8.1 ImGui 源码归属

NNRuntimeImGui 的 ImGui 版本优先。NNRuntimeDiligent 停止编译 Diligent 捆绑的 ImGui，改为使用 NNRuntimeImGui 的版本。

```
NNRuntimeImGui (SHARED)
  ├── 编译 ImGui 核心（版本优先）
  └── 编译扩展（ImGuiEx, ImNodeEditor, imGuizmo, Imnodes, ImGuiColorTextEdit）

NNRuntimeDiligent (STATIC)
  ├── 编译 Diligent ImGui 后端（ImGuiImplDiligent）
  └── include 指向 NNRuntimeImGui 的 imgui.h
```

### 8.2 CMake 变更

**文件**: `Runtime/NNRuntimeImGui/CMakeLists.txt`

```cmake
# 移除
# link_libraries(opengl32)
# set(IMGUI_BACKEND_SOURCES ...)  # imgui_impl_opengl3.cpp

# 添加 Diligent 后端
link_libraries(NNRuntimeDiligent)
```

**文件**: `Runtime/NNRuntimeDiligent/CMakeLists.txt`

```cmake
# 停止编译 Diligent 捆绑的 ImGui 核心
# include 路径指向 NNRuntimeImGui 的 ImGui
target_include_directories(NNRuntimeDiligent PRIVATE
    ${NNRUNTIME_IMGUI_INCLUDE_DIR}
)
```

### 8.3 ImGui 层

**文件**: `Runtime/NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h`

```cpp
// 移除 Opengl3ImGuiWindowLayer
// 添加 DiligentImGuiWindowLayer
class DiligentImGuiWindowLayer : public IWindowLayer {
public:
    DiligentImGuiWindowLayer(Window* window, INNRenderDevice* device);
    void BeginFrame() override;
    void EndFrame() override;
    void HandleEvent(const SDL_Event& event) override;
private:
    std::unique_ptr<NNDiligentImGuiRenderer> m_Renderer;
};
```

### 8.4 验收标准

- [ ] NNRuntimeImGui 无 OpenGL 头文件
- [ ] ImGui 核心来自 NNRuntimeImGui（版本优先）
- [ ] Diligent ImGui 后端正常工作
- [ ] ImGui 扩展（ImNodeEditor, ImGuizmo 等）正常
- [ ] 无重复符号链接错误
- [ ] ImGuiContext 正确共享

---

## 9. Phase 5：Application + SwapChain ✅

**目标**: 替换 SDL_GL_CreateContext，用 Diligent 管理窗口生命周期

### 9.1 实际修改文件

| 文件 | 变更 |
|------|------|
| `Runtime/NNRuntimeApplication/CMakeLists.txt` | 依赖替换：RHI → NNRuntimeRender + NNRuntimeDiligent + NNRuntimeRenderBootstrap + NevernessRuntime-RenderAssets |
| `Runtime/NNRuntimeApplication/Private/Core/Window.cpp` | Diligent 设备创建 + RenderAssetManager 初始化 |
| `Runtime/NNRuntimeApplication/Private/Engine/ImGuiLayer.cpp` | Diligent ImGui 后端 + SetRenderTargets 修复 |
| `Runtime/NNRuntimeApplication/Private/RuntimeApplication.cpp` | SwapChain Present + ImGui ini 恢复 |
| `Runtime/NNRuntimeRenderAssets/Include/NNTextureResource.h` | NNTextureDesc → NNTextureCacheDesc（避免与 Experiments 重名） |
| `Runtime/NNRuntimeRenderAssets/Include/NNRenderAssetManager.h` | Initialize(unique_ptr) + m_Factory 持有所有权 |
| `Runtime/NNRuntimeRenderAssets/Include/DiligentRenderResourceFactory.h` | **新建** — IRenderResourceFactory 的 Diligent 实现 |
| `Runtime/NNRuntimeRenderAssets/Private/DiligentRenderResourceFactory.cpp` | **新建** — 通过 INNRenderDevice 创建纹理 |
| `Runtime/NNRuntimeRenderAssets/Private/NNRenderAssetManager.cpp` | Initialize 签名改 unique_ptr |
| `Runtime/NNRuntimeEngineServices/.../NativeEngineRuntimeApiTable.cpp` | 移除 Initialize(nullptr) |
| `Experiments/NNRuntimeRender/Resources/INNTexture.h` | +GetShaderResourceView() 虚方法 |
| `Experiments/NNRuntimeDiligent/Resources/NNDiligentTexture.h` | +override GetShaderResourceView |
| `Experiments/NNRuntimeDiligent/Source/Resources/NNDiligentTexture.cpp` | 实现 GetShaderResourceView（返回 SRV） |
| `Experiments/NNRuntimeDiligent/Source/Device/NNDiligentDevice.cpp` | CreateTexture 支持 initialData |

### 9.2 关键修复

1. **RenderAssetManager 延迟初始化**: 设备创建后才调用 Initialize(factory)，不再传 nullptr
2. **ImGui SetRenderTargets**: EndFrame() 中在 Render 之前绑定 SwapChain RTV/DSV
3. **ImGui ini 文件**: 恢复 io.IniFilename（Diligent 后端构造函数置空了）
4. **NNTextureDesc 重命名**: Runtime 版改名 NNTextureCacheDesc，避免与 Experiments 版冲突
5. **CreateTexture initialData**: NNDiligentDevice 正确传递像素数据给 Diligent

### 9.3 验收标准

- [x] 窗口不创建 OpenGL 上下文
- [x] Diligent 设备通过 NNRenderBootstrap 创建
- [x] SwapChain Present 替换 SDL_GL_SwapWindow
- [x] RenderAssetManager 正确初始化（Diligent 工厂）
- [x] ImGui 渲染正常（SetRenderTargets 修复）
- [x] 主窗口正常显示

---

## 10. Phase 6：ViewportRender 🔧

**目标**: C# Editor 通过 uint64_t 访问纹理

### 10.1 实际修改文件

| 文件 | 变更 |
|------|------|
| `Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | 从主窗口获取设备；修复 textureId 类型截断 |
| `Runtime/NNRuntimeEngineServices/CMakeLists.txt` | 添加 Experiments include 路径 |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/Renderer2D.h` | +SetRenderTarget() 方法 |
| `Runtime/NNRuntimeRenderer2D/Source/Renderer2D.cpp` | BeginRenderPass/EndRenderPass；diliPSO/diliRenderPass 改 RefCntAutoPtr |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/FramebufferObject.h` | +GetColorRTV()/GetDepthDSV() |
| `Runtime/NNRuntimeRenderer2D/Source/FramebufferObject.cpp` | GetColorTextureHandle 改用 SRV；修复 Resize m_Device 清空 bug |
| `Runtime/NNRuntimeRenderer2D/Source/SceneRenderer.cpp` | 渲染前调用 SetRenderTarget |
| `Runtime/NNRuntimeRenderer2D/CMakeLists.txt` | 添加 DiligentCore include 路径 |
| `Experiments/NNRuntimeDiligent/Resources/NNDiligentRenderTarget.h` | +m_ColorSRVs +GetColorSRV() |
| `Experiments/NNRuntimeDiligent/Source/Resources/NNDiligentRenderTarget.cpp` | 初始化时创建 SRV |

### 10.2 关键修复

1. **ViewportRender 设备传递**: 从 WindowRegistry 获取主窗口设备，传给 SceneRenderer/RmlUIRenderer
2. **textureId 类型截断**: `uint32_t` → `uint64_t`
3. **RenderPass 缺失**: PSO 关联了 RenderPass，必须 BeginRenderPass/EndRenderPass
4. **Framebuffer 创建**: SetRenderTarget 从 RTV/DSV 创建 Diligent Framebuffer
5. **SRV vs RTV**: ImGui 需要 SRV 采样，GetColorTextureHandle 改用 GetColorSRV
6. **FBO Resize bug**: Shutdown 清空 m_Device，Resize 后 Initialize 收到 null
7. **悬空指针**: diliPSO/diliRenderPass 从裸指针改为 RefCntAutoPtr

### 10.3 验收标准

- [ ] ViewportRender 返回 uint64_t（SRV 指针）
- [ ] C# Editor 正确显示 Scene Viewport
- [ ] C# Editor 正确显示 RmlUI Viewport
- [ ] Editor 不包含任何 Diligent 头文件
- [ ] 无堆损坏崩溃
- [ ] 无内存泄漏

---

## 11. Phase 7：验证

**目标**: 全链路验证，确认迁移完整性

### 11.1 验证清单

```
渲染链路：
  [ ] Scene Viewport 正确渲染
  [ ] RmlUI Viewport 正确渲染
  [ ] Renderer2D Sprite 正确显示
  [ ] ImGui 界面正常

功能验证：
  [ ] 纹理创建/更新正常
  [ ] Shader 编译正常（GLSL）
  [ ] 帧率无明显退化

构建验证：
  [ ] 全项目编译通过
  [ ] 无 OpenGL 头文件泄漏到迁移模块
  [ ] 无重复符号
```

### 11.2 验收标准

- [ ] 所有验证项通过
- [ ] 无已知回归

---

## 12. Phase 8：RHI Deprecated（不删）

**目标**: NNRuntimeRHI 标记 Deprecated，保留至少一个版本

### 12.1 标记 Deprecated

```cpp
// NNRuntimeRHI/Interface/VGFX.h
[[deprecated("Use NNRuntimeRender interfaces instead")]]
namespace VGFX { ... }
```

### 12.2 不立刻删除

原因：
- NNEngineLegacy 可能有隐藏引用
- Editor/Tools 可能有间接依赖
- 至少一个版本后再清理

### 12.3 清理时间线

```
v1: 迁移完成，RHI Deprecated
v2: 全部验证通过
v3: 删除 RHI + 清理 OpenGL 代码
```

### 12.4 验收标准

- [ ] NNRuntimeRHI 标记 Deprecated
- [ ] 编译警告提示使用新接口
- [ ] 不删除任何文件

---

## 13. NNEngineLegacy：完全冻结

**原则**: 不删、不改、不动。

- 不删 `opengl32` 链接
- 不脱钩、不条件编译
- 不做任何 CMake 变更
- 等主链路迁移成功并稳定运行后，再考虑 Legacy 处理

---

## 14. 依赖顺序

```
Phase 0:   构建系统
   │
Phase 0.5: ImGui 版本验证
   │
Phase 1:   RenderAssets
   │
Phase 2:   Renderer2D
   │
Phase 3:   RmlDiligent 接入
   │
Phase 4:   ImGui 后端切换
   │
Phase 5:   Application + SwapChain
   │
Phase 6:   ViewportRender
   │
Phase 7:   验证
   │
Phase 8:   RHI Deprecated

NNEngineLegacy: 完全冻结，不参与迁移
```

Phase 0-5 已全部完成。
Phase 6（ViewportRender）是当前目标：C# Editor 纹理句柄。

---

## 15. 简化方案

如果只需要最小迁移：

```
保留（不改）：
  NNRuntimeRender
  NNRuntimeDiligent
  NNRuntimeRenderBootstrap

迁移（改）：
  NNRuntimeRenderAssets → IRenderResourceFactory
  NNRuntimeRenderer2D → NNRuntimeRender 接口
  NNRuntimeRmlui → RmlDiligent 直接接入
  NNRuntimeImGui → Diligent ImGui 后端（版本对齐）
  NNRuntimeApplication → Diligent Device + SwapChain

直接复用（不改）：
  RmlDiligent

完全冻结（不动）：
  NNEngineLegacy
  NNRuntimeRHI → Deprecated
```

核心迁移量约 40%（RenderAssets + Renderer2D + ImGui + Application）。

---

## 16. 风险与对策

| 风险 | 影响 | 对策 | 状态 |
|------|------|------|------|
| ~~GLSL 编译兼容性~~ | ~~低~~ | ~~Diligent 原生支持 GLSL~~ | ✅ 已解决：转为 HLSL |
| Experiments 接口不足 | 中 | Runtime 侧用 raw Diligent 补充（PSO 创建、SRB 绑定） | ✅ Phase 2 已验证 |
| ~~ImGui 版本不一致~~ | ~~高~~ | ~~Phase 0.5 预检查，以 NNRuntimeImGui 版本为准~~ | ✅ Phase 4 已解决 |
| ~~ImGui 重复符号~~ | ~~高~~ | ~~NNRuntimeDiligent 停止编译核心，使用 NNRuntimeImGui 版本~~ | ✅ Phase 4 已解决 |
| ~~RenderAssetManager 初始化时机~~ | ~~高~~ | ~~延迟到设备创建后~~ | ✅ Phase 5 已解决 |
| ~~NNTextureDesc 重名冲突~~ | ~~中~~ | ~~Runtime 版改名 NNTextureCacheDesc~~ | ✅ Phase 5 已解决 |
| ~~RenderPass 悬空指针~~ | ~~高~~ | ~~diliPSO/diliRenderPass 改 RefCntAutoPtr~~ | ✅ Phase 6 已解决 |
| ~~SRV vs RTV 混淆~~ | ~~高~~ | ~~GetColorTextureHandle 改用 GetColorSRV~~ | ✅ Phase 6 已解决 |
| ~~FBO Resize m_Device 清空~~ | ~~中~~ | ~~Resize 先保存 device 再 Shutdown~~ | ✅ Phase 6 已解决 |
| 静态库 ABI 不匹配 | 高 | NNDiligentRenderTarget 类布局变化需全部重新编译 | ⚠️ 待验证 |
| NNEngineLegacy 隐藏引用 | 无 | 完全冻结，不碰 | — |

---

## 17. 关键文件清单

| 文件 | Phase | 变更类型 |
|------|-------|---------|
| `Engine/Source/CMakeLists.txt` | 0 | 构建顺序 |
| ImGui 版本检查（各库 version.h） | 0.5 | 版本验证 |
| `Runtime/NNRuntimeRenderAssets/CMakeLists.txt` | 1 | 依赖替换 |
| `Runtime/NNRuntimeRenderAssets/Private/NNRenderAssetManager.cpp` | 1 | 工厂模式重写 |
| `Runtime/NNRuntimeRenderer2D/CMakeLists.txt` | 2 | 依赖替换 |
| `Runtime/NNRuntimeRenderer2D/Source/Renderer2D.cpp` | 2 | 核心重写 |
| `Runtime/NNRuntimeRenderer2D/Source/FramebufferObject.cpp` | 2 | FBO→RenderTarget |
| `Runtime/NNRuntimeRenderer2D/Source/SceneRenderer.cpp` | 2 | 渲染流程更新 |
| `Runtime/NNRuntimeRmlui/CMakeLists.txt` | 3 | 依赖替换 |
| `Runtime/NNRuntimeRmlui/Include/Renderer/RmlUIRenderer.h` | 3 | 后端替换 |
| `Runtime/NNRuntimeRmlui/Source/Renderer/RmlUIRenderer.cpp` | 3 | 核心重写 |
| `Runtime/NNRuntimeImGui/CMakeLists.txt` | 4 | 源码裁剪+依赖替换 |
| `Runtime/NNRuntimeImGui/Include/ImGuiLayer/SDL3Decorator.h` | 4 | 新增 Diligent 层 |
| `Runtime/NNRuntimeDiligent/CMakeLists.txt` | 4 | ImGui include 对齐 |
| `Runtime/NNRuntimeApplication/CMakeLists.txt` | 5 | 依赖替换 |
| `Runtime/NNRuntimeApplication/Private/Core/Window.cpp` | 5 | 设备创建+RenderAssetManager 初始化 |
| `Runtime/NNRuntimeApplication/Private/Engine/ImGuiLayer.cpp` | 5 | Diligent ImGui 后端+SetRenderTargets |
| `Runtime/NNRuntimeApplication/Private/RuntimeApplication.cpp` | 5 | SwapChain Present+ImGui ini |
| `Runtime/NNRuntimeRenderAssets/Include/DiligentRenderResourceFactory.h` | 5 | 新建：IRenderResourceFactory 实现 |
| `Runtime/NNRuntimeRenderAssets/Private/DiligentRenderResourceFactory.cpp` | 5 | 新建：工厂实现 |
| `Runtime/NNRuntimeRenderAssets/Include/NNTextureResource.h` | 5 | NNTextureDesc→NNTextureCacheDesc |
| `Runtime/NNRuntimeRenderAssets/Include/NNRenderAssetManager.h` | 5 | Initialize(unique_ptr) |
| `Experiments/NNRuntimeRender/Resources/INNTexture.h` | 5 | +GetShaderResourceView() |
| `Experiments/NNRuntimeDiligent/Resources/NNDiligentTexture.h` | 5 | +override |
| `Experiments/NNRuntimeDiligent/Source/Resources/NNDiligentTexture.cpp` | 5 | 实现 GetShaderResourceView |
| `Experiments/NNRuntimeDiligent/Source/Device/NNDiligentDevice.cpp` | 5 | CreateTexture 支持 initialData |
| `Runtime/NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | 6 | 设备传递+textureId 类型修复 |
| `Runtime/NNRuntimeEngineServices/CMakeLists.txt` | 6 | Experiments include 路径 |
| `Runtime/NNRuntimeRenderer2D/Include/Renderer2D/Renderer2D.h` | 6 | +SetRenderTarget() |
| `Runtime/NNRuntimeRenderer2D/Source/Renderer2D.cpp` | 6 | BeginRenderPass/EndRenderPass + RefCntAutoPtr |
| `Runtime/NNRuntimeRenderer2D/Source/FramebufferObject.cpp` | 6 | SRV 获取 + Resize 修复 |
| `Runtime/NNRuntimeRenderer2D/Source/SceneRenderer.cpp` | 6 | SetRenderTarget 调用 |
| `Experiments/NNRuntimeDiligent/Resources/NNDiligentRenderTarget.h` | 6 | +m_ColorSRVs +GetColorSRV() |
| `Experiments/NNRuntimeDiligent/Source/Resources/NNDiligentRenderTarget.cpp` | 6 | SRV 创建 |
| `Runtime/NNRuntimeRHI/Interface/VGFX.h` | 8 | 标记 Deprecated |

NNEngineLegacy 不在清单中 — 完全冻结，不修改任何文件。

---

> **下一步**: Phase 6 ViewportRender — C# Editor 通过 uint64_t 访问纹理。
