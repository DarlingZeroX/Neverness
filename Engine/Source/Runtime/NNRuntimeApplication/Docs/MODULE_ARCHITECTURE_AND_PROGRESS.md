# NNRuntimeApplication

## 职责

- SDL 生命周期（`SDL_Init` / `SDL_Quit`）
- 主窗口（`SDL_Window`）
- 事件泵（`SDL_PollEvent`，`SDL_EVENT_QUIT` → 退出循环）
- 导出 `NNApplicationAPI`（经 `NNBuildApplicationRuntimeApi` 挂入 `NNNativeEngineAPI`）

## 不负责

- Gameplay、Scene Runtime、ECS、Editor UI、Render Backend（Managed / `NNEngineRuntime`）

## 依赖

- `NNNativeEngineAPI`（`ApplicationAPI.h`）
- SDL3

## 进度

- [x] Phase 1：`RuntimeApplication` 最小 SDL 宿主
- [x] Phase 2：挂入 layout v6 `NNNativeEngineAPI.application`
