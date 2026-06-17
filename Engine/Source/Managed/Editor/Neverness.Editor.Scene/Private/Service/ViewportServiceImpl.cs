using System.Numerics;
using System.Runtime.InteropServices;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Rendering.Diligent.Commands;
using SpriteFlags = Neverness.Runtime.Scene.Components.SpriteFlags;

namespace Neverness.Editor.Scene.Private.Service;

/// <summary>
/// 视口服务实现——封装 Native 渲染 API。
/// 对外暴露 IViewportService 接口，Controller 不直接访问 Native API。
/// </summary>
public sealed unsafe class ViewportServiceImpl : IViewportService
{
    private SceneWorld? _scene;

    /// <summary>当前关联的场景。</summary>
    public SceneWorld? Scene => _scene;

    /// <summary>是否有有效的场景。</summary>
    public bool HasScene => _scene != null;

    /// <summary>设置关联的场景。</summary>
    public void SetScene(SceneWorld? scene)
    {
        _scene = scene;
    }

    /// <summary>渲染场景到纹理并返回纹理 ID。</summary>
    public ulong RenderSceneToTexture(float width, float height)
    {
        if (_scene == null || width < 1 || height < 1) return 0;

        // TODO: 更新为使用新的 Scene API
        // 暂时返回 0
        return 0;
    }

    /// <summary>获取最后渲染的场景纹理 ID。</summary>
    public ulong GetLastSceneTextureId()
    {
        // TODO: 更新为使用新的 Scene API
        return 0;
    }

    /// <summary>获取最后渲染的 RmlUI 纹理 ID。</summary>
    public ulong GetLastRmluiTextureId()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.GetLastRmluiTexture == null) return 0;

        return api.ViewportRender.GetLastRmluiTexture();
    }

    /// <summary>聚焦到指定实体（摄像机移动）。</summary>
    /// <remarks>
    /// Native API 没有 FocusEntity 函数指针。
    /// 需要 Native 端在 NNViewportRenderApi 中添加 FocusEntity 接口。
    /// </remarks>
    public void FocusEntity(IEntity entity)
    {
        // Native API 不支持摄像机聚焦
        // NNViewportRenderApi 没有 FocusEntity 函数指针
        Console.WriteLine($"[ViewportService] FocusEntity 不支持: {entity.Id} (Native API 无接口)");
    }

    /// <summary>设置摄像机位置。</summary>
    /// <remarks>
    /// Native API 没有 SetCameraPosition 函数指针。
    /// 需要 Native 端在 NNViewportRenderApi 中添加 SetCameraPosition 接口。
    /// </remarks>
    public void SetCameraPosition(float x, float y, float z)
    {
        // Native API 不支持设置摄像机位置
        // NNViewportRenderApi 没有 SetCameraPosition 函数指针
        Console.WriteLine($"[ViewportService] SetCameraPosition 不支持: ({x}, {y}, {z}) (Native API 无接口)");
    }

    /// <summary>获取渲染统计信息。</summary>
    public RenderStats GetRenderStats()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;

        uint drawCalls = 0;
        uint vertices = 0;

        if (api.ViewportRender.GetRenderStats != null)
        {
            api.ViewportRender.GetRenderStats(&drawCalls, &vertices);
        }

        return new RenderStats
        {
            DrawCalls = drawCalls,
            Vertices = vertices,
            Triangles = vertices / 3 // 估算
        };
    }

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    public void SetRmlUIViewportSize(uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.SetRmlUIViewportSize != null)
        {
            api.ViewportRender.SetRmlUIViewportSize(width, height);
        }
    }

    /// <summary>
    /// 从 ECS 收集渲染命令并序列化为 Flat Buffer。
    ///
    /// 数据流：
    /// Friflo ECS → SetCamera + DrawSpriteBatch → RenderCommandBuffer → byte[]
    /// </summary>
    public byte[]? CollectRenderCommands(float width, float height)
    {
        if (_scene == null || width < 1 || height < 1)
            return null;

        var buffer = new RenderCommandBuffer();

        // ── 1. 收集相机数据（取第一个 CameraComponent） ──
        bool hasCamera = false;
        var cameraView = _scene.Scene.CreateView<TransformComponent, CameraComponent>();
        cameraView.Refresh();
        cameraView.ForEach(
            (IEntity entity, TransformComponent transform, CameraComponent camera) =>
            {
                if (hasCamera) return; // 只取第一个

                // 计算 ViewMatrix = 逆 WorldMatrix
                var worldMatrix = transform.WorldMatrix;
                // 相机 Z 偏移（与 C++ SceneRenderer 一致：避免近平面裁剪）
                float nearZ = camera.NearPlane;
                float farZ = camera.FarPlane;
                float cameraZ = (nearZ + farZ) * 0.5f;
                worldMatrix.M43 = cameraZ;

                Matrix4x4.Invert(worldMatrix, out var viewMatrix);

                // ProjectionMatrix（由 CameraSystem 每帧计算）
                var projMatrix = camera.ProjectionMatrix;

                // System.Numerics 是行主序，Diligent/GLM 是列主序，需要转置
                var viewT = Matrix4x4.Transpose(viewMatrix);
                var projT = Matrix4x4.Transpose(projMatrix);
                unsafe
                {
                    buffer.AddSetCamera(
                        new ReadOnlySpan<float>(&viewT, 16),
                        new ReadOnlySpan<float>(&projT, 16),
                        width, height,
                        camera.NearPlane, camera.FarPlane,
                        camera.OrthographicSize * camera.AspectRatio,
                        camera.OrthographicSize);
                }

                hasCamera = true;
            });

        // 如果没有相机，使用默认正交相机
        if (!hasCamera)
        {
            var identity = Matrix4x4.Identity;
            unsafe
            {
                buffer.AddSetCamera(
                    new ReadOnlySpan<float>(&identity, 16),
                    new ReadOnlySpan<float>(&identity, 16),
                    width, height, -1f, 1f,
                    width, height);
            }
        }

        // ── 2. 设置渲染 Pass 状态 ──
        ReadOnlySpan<float> clearColor = stackalloc float[] { 0.1f, 0.1f, 0.1f, 1f };
        buffer.AddSetRenderPassState(clearColor, RenderPassFlags.ClearColor | RenderPassFlags.DepthTest);

        // ── 3. 收集精灵数据 ──
        var sprites = new List<SpriteInstance>();
        var spriteView = _scene.Scene.CreateView<TransformComponent, SpriteRendererComponent>();
        spriteView.Refresh();
        spriteView.ForEach(
            (IEntity entity, TransformComponent transform, SpriteRendererComponent sprite) =>
            {
                // 跳过不可见的精灵
                if ((sprite.Flags & SpriteFlags.Visible) == 0)
                    return;

                var instance = new SpriteInstance();
                unsafe
                {
                    // WorldMatrix（4x4 列主序）
                    // System.Numerics Matrix4x4 是行主序，需要转置
                    var transposed = Matrix4x4.Transpose(transform.WorldMatrix);
                    var matBytes = new byte[64];
                    MemoryMarshal.Write(matBytes.AsSpan(), in transposed);
                    fixed (byte* src = matBytes)
                    {
                        Buffer.MemoryCopy(src, instance.Transform, 64, 64);
                    }

                    // 颜色
                    instance.Color[0] = sprite.ColorR;
                    instance.Color[1] = sprite.ColorG;
                    instance.Color[2] = sprite.ColorB;
                    instance.Color[3] = sprite.ColorA;

                    // UV
                    instance.UvRect[0] = sprite.UvU0;
                    instance.UvRect[1] = sprite.UvV0;
                    instance.UvRect[2] = sprite.UvU1;
                    instance.UvRect[3] = sprite.UvV1;
                }

                instance.TextureHandle = 0; // TODO: 从 TextureAsset GUID 解析 GPU Handle
                instance.Layer = sprite.Layer;
                instance.SortOrder = sprite.SortOrder;
                instance.BlendMode = (uint)sprite.Blend;
                instance.Flags = 0;
                if ((sprite.Flags & SpriteFlags.FlipX) != 0) instance.Flags |= (uint)SpriteFlags.FlipX;
                if ((sprite.Flags & SpriteFlags.FlipY) != 0) instance.Flags |= (uint)SpriteFlags.FlipY;

                sprites.Add(instance);
            });

        if (sprites.Count > 0)
        {
            buffer.AddDrawSpriteBatch(CollectionsMarshal.AsSpan(sprites));
        }

        // ── 4. 构建并返回 ──
        return buffer.Build();
    }

}
