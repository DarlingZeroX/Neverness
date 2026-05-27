/**
 * @file SpriteRenderSystem.cpp
 * @brief ECS System 实现：从 Scene 收集 SpriteDrawCommand。
 */

#include "Renderer2D/SpriteRenderSystem.h"
#include <Scene/NNRuntimeScene.h>
#include <Scene/NNWorld.h>
#include <Components/NNTransformComponent.h>
#include <Components/NNSpriteRendererComponent.h>
#include <NNRuntimeRenderAssets/Include/NNRenderAssetManager.h>
#include <NNRuntimeAsset/Include/NNAssetManager.h>
#include <algorithm>
#include <cstring>

#include "NNCore/Interface/HLog.h"

namespace NN::Runtime::Renderer2D
{
    void SpriteRenderSystem::Collect(
        Scene::NNRuntimeScene& scene,
        std::vector<SpriteDrawCommand>& outCommands)
    {
        outCommands.clear();

        // entt 视图：查询同时拥有 Transform + SpriteRenderer 的实体
        auto& registry = scene.GetRegistry();
        auto view = registry.view<
            Scene::NNTransformComponent,
            Scene::NNSpriteRendererComponent>();

        for (auto [entity, transform, sprite] : view.each())
        {
            // 跳过不可见
            if (!Scene::HasFlag(sprite.Flags, Scene::NNSpriteFlags::Visible))
                continue;

            // 懒解析：TextureAsset (GUID.Low) → TextureRuntimeId (GL texture ID)
            H_LOG_INFO("[Collect] Sprite: TextureAsset=%llu TextureRuntimeId=%u",
                       sprite.TextureAsset, sprite.TextureRuntimeId);
            if (sprite.TextureRuntimeId == 0 && sprite.TextureAsset != 0)
            {
                auto& renderMgr = NN::Runtime::Render::NNRenderAssetManager::Get();
                uint64_t cacheKey = renderMgr.GetCacheKeyByGuidLow(sprite.TextureAsset);
                H_LOG_INFO("[Collect] GetCacheKeyByGuidLow(%llu) → cacheKey=%llu", sprite.TextureAsset, cacheKey);
                if (cacheKey == 0)
                {
                    // 首次访问：加载资产 → 上传 GPU → 注册 GUID 映射
                    auto& assetMgr = NN::Runtime::Asset::NNAssetManager::Instance();
                    auto handle = assetMgr.LoadAssetByGuidLow(sprite.TextureAsset, 1);
                    H_LOG_INFO("[Collect] LoadAssetByGuidLow(%llu) → handle=%llu", sprite.TextureAsset, handle ? handle.Value() : 0);
                    if (handle)
                        cacheKey = renderMgr.LoadTextureFromAsset(handle.Value(), sprite.TextureAsset);
                    H_LOG_INFO("[Collect] LoadTextureFromAsset → cacheKey=%llu", cacheKey);
                }
                if (cacheKey != 0)
                {
                    auto glId = renderMgr.GetGLTextureId(cacheKey);
                    sprite.TextureRuntimeId = static_cast<std::uint32_t>(glId);
                    H_LOG_INFO("[Collect] GetGLTextureId(%llu) → glId=%llu → TextureRuntimeId=%u",
                               cacheKey, glId, sprite.TextureRuntimeId);
                }
            }

            SpriteDrawCommand cmd{};

            // 拷贝 WorldMatrix（4x4 列主序）
            std::memcpy(cmd.Transform, &transform.WorldMatrix, sizeof(float) * 16);

            // 纹理：直接使用已解析的 GL texture ID（0 = 使用白色默认纹理）
            cmd.TextureHandle = sprite.TextureRuntimeId;

            // 颜色
            cmd.Color[0] = sprite.Color[0];
            cmd.Color[1] = sprite.Color[1];
            cmd.Color[2] = sprite.Color[2];
            cmd.Color[3] = sprite.Color[3];

            // UV Rect
            cmd.UvRect[0] = sprite.UvRect[0];
            cmd.UvRect[1] = sprite.UvRect[1];
            cmd.UvRect[2] = sprite.UvRect[2];
            cmd.UvRect[3] = sprite.UvRect[3];

            // 排序
            cmd.Layer     = sprite.Layer;
            cmd.SortOrder = sprite.SortOrder;

            // 混合模式
            cmd.Blend = static_cast<BlendMode>(
                static_cast<std::uint32_t>(sprite.Blend));

            // Flip
            cmd.FlipX = Scene::HasFlag(sprite.Flags, Scene::NNSpriteFlags::FlipX);
            cmd.FlipY = Scene::HasFlag(sprite.Flags, Scene::NNSpriteFlags::FlipY);

            outCommands.push_back(cmd);
        }

        SortCommands(outCommands);
    }

    void SpriteRenderSystem::SortCommands(std::vector<SpriteDrawCommand>& commands)
    {
        std::sort(commands.begin(), commands.end(),
            [](const SpriteDrawCommand& a, const SpriteDrawCommand& b)
            {
                if (a.Layer != b.Layer)
                    return a.Layer < b.Layer;
                return a.SortOrder < b.SortOrder;
            });
    }
}
