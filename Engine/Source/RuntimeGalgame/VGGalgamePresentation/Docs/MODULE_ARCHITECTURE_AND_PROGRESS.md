# VGGalgamePresentation — 表现层（Phase 8）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 渲染管线、转场、对白 UI、立绘动画等 **与玩法状态解耦** 的表现逻辑。 |
| **当前内容** | `RenderPipeline`（自 `VGGalgame` 迁入）。 |
| **CMake** | `SHARED`；宏 **`VG_GALGAME_PRESENTATION_API`**；`PUBLIC` 链接 **`VGGalgameCore`**、**`VGEngine`**。 |

## 2. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-13 | Phase 8.4：创建模块并迁入 **`RenderPipeline`**；`VGGalgame/Include/RenderPipeline.h` 为薄转发。 |
