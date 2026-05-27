#pragma once

/**
 * @file SpriteRenderSystem.h
 * @brief ECS System：遍历 Scene 中 Transform + SpriteRenderer，收集 SpriteDrawCommand。
 *
 * 不直接调用 OpenGL，只负责数据收集和排序。
 */

#include "SpriteDrawCommand.h"
#include "Renderer2DExport.h"
#include <vector>

namespace NN::Runtime::Scene
{
    class NNRuntimeScene;
}

namespace NN::Runtime::Renderer2D
{
    /// Sprite 绘制命令收集器
    class NN_RUNTIME_RENDERER2D_API SpriteRenderSystem
    {
    public:
        /// 从 Scene 中收集所有可见 Sprite 的绘制命令
        void Collect(Scene::NNRuntimeScene& scene,
                     std::vector<SpriteDrawCommand>& outCommands);

    private:
        /// 排序：先 Layer 升序，再 SortOrder 升序
        static void SortCommands(std::vector<SpriteDrawCommand>& commands);
    };
}
