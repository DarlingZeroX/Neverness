# NNRuntimeApplication

## 职责

- SDL 子系统生命周期（`SDL_Init` / `SDL_Quit`，经 **NNApplicationAPI**）
- 全局事件泵（`SDL_PollEvent`，`SDL_EVENT_QUIT` / 窗口关闭 → 退出循环）
- 帧边界（`beginFrame` / `endFrame`，ImGui + GL Swap）
- 窗口注册表（**WindowRegistry**）与 **NNWindowAPI** 导出（`NNBuildWindowRuntimeApi`）
- **VGWindow**：SDL3 + OpenGL 宿主（Editor 路径）

## 不负责

- Gameplay、Scene Runtime、ECS、Editor UI 业务（Managed / `NNEngineRuntime`）
- ABI 契约定义（见 **NNNativeEngineAPI**）

## 依赖

- `NNNativeEngineAPI`（`ApplicationAPI.h`、`WindowAPI.h`）
- SDL3、NNPlatformCore、NNRuntimeImGui

## 关键文件

| 文件 | 说明 |
|------|------|
| `Private/BuildApplicationApi.cpp` | **NNApplicationAPI** 函数表 |
| `Private/BuildWindowApi.cpp` | **NNWindowAPI** 函数表 |
| `Private/Core/WindowRegistry.cpp` | 原子句柄 ↔ `VGWindow` |
| `Private/RuntimeApplication.cpp` | Host 生命周期与主窗口帧泵 |

## 进度

- [x] Phase 1：`RuntimeApplication` SDL Host
- [x] Phase 2：layout v6 `NNNativeEngineAPI.application`
- [x] Phase 3：**layout v7** — Application 瘦身 + **NNWindowAPI** + **WindowRegistry**
