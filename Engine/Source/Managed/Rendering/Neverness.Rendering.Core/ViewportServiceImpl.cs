using System.Numerics;
using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;
using Neverness.Rendering.Diligent.Commands;
using Neverness.Rendering.RenderAssets;
using SpriteFlags = Neverness.Runtime.Scene.Components.SpriteFlags;

namespace Neverness.Rendering.Core;

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

    /// <summary>
    /// 资产 GUID → VFSService 路径解析器（由上层注入，避免 Editor.Scene 直接依赖 Editor.Assets）。
    /// 返回 null 表示解析失败。
    /// </summary>
    public Func<NNGuid, string?>? AssetPathResolver { get; set; }

    /// <summary>设置关联的场景。</summary>
    public void SetScene(SceneWorld? scene)
    {
        _scene = scene;
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

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    public void SetRmlUIViewportSize(uint width, uint height)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.SetRmlUIViewportSize != null)
        {
            api.ViewportRender.SetRmlUIViewportSize(width, height);
        }
    }

    // 调试日志节流：每 120 帧打印一次（约 2 秒 @60fps）
    private int _debugFrameCounter;
    private const int DebugLogInterval = 120;

    /// <summary>
    /// 从 ECS 收集渲染命令并序列化为 Flat Buffer。
    ///
    /// 数据流：
    /// Friflo ECS → SetCamera + DrawSpriteBatch → RenderCommandBuffer → byte[]
    /// </summary>
    public byte[]? CollectRenderCommands(float width, float height)
    {
        _debugFrameCounter++;
        bool shouldLog = (_debugFrameCounter % DebugLogInterval == 1);

        if (_scene == null)
        {
            if (shouldLog) Console.WriteLine("[ViewportService] CollectRenderCommands: _scene = null");
            return null;
        }

        if (width < 1 || height < 1)
        {
            if (shouldLog) Console.WriteLine($"[ViewportService] CollectRenderCommands: 尺寸无效 {width}x{height}");
            return null;
        }

        var buffer = new RenderCommandBuffer();

        // ── 1. 收集相机数据（取第一个 CameraComponent） ──
        bool hasCamera = false;
        var cameraView = _scene.Scene.CreateView<TransformComponent, CameraComponent>();
        cameraView.Refresh();
        int cameraCount = 0;

        cameraView.ForEach(
            (IEntity entity, TransformComponent transform, CameraComponent camera) =>
            {
                cameraCount++;
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

                // System.Numerics 行主序内存布局 = HLSL 列主序 float4x4 布局（不需要转置）
                // 行主序 [M11..M14, M21..M24, M31..M34, M41..M44]
                // HLSL 读为 Col0=[M11..M14], Col1=[M21..M24], ..., Col3=[M41..M44]
                // 平移 M41/M42/M43 自动成为 M[0][3]/M[1][3]/M[2][3]（正确的列主序平移位置）
                unsafe
                {
                    buffer.AddSetCamera(
                        new ReadOnlySpan<float>(&viewMatrix, 16),
                        new ReadOnlySpan<float>(&projMatrix, 16),
                        width, height,
                        camera.NearPlane, camera.FarPlane,
                        camera.OrthographicSize * camera.AspectRatio,
                        camera.OrthographicSize);
                }

                if (shouldLog)
                {
                    Console.WriteLine($"[ViewportService] Camera: pos=({transform.Position.X:F2}, {transform.Position.Y:F2}, {transform.Position.Z:F2}), " +
                        $"ortho={camera.IsOrthographic}, fov={camera.FieldOfView:F2}, near={camera.NearPlane:F2}, far={camera.FarPlane:F2}, " +
                        $"projMatrix=[{projMatrix.M11:F3}, {projMatrix.M22:F3}, {projMatrix.M33:F3}, {projMatrix.M44:F3}], " +
                        $"worldMatrix.M43={transform.WorldMatrix.M43:F3}");
                }

                hasCamera = true;
            });

        if (shouldLog)
            Console.WriteLine($"[ViewportService] 场景相机数量: {cameraCount}");

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

            if (shouldLog)
                Console.WriteLine("[ViewportService] 无相机实体，使用默认单位矩阵相机");
        }

        // ── 2. 设置渲染 Pass 状态 ──
        ReadOnlySpan<float> clearColor = stackalloc float[] { 0.1f, 0.1f, 0.1f, 1f };
        buffer.AddSetRenderPassState(clearColor, RenderPassFlags.ClearColor | RenderPassFlags.DepthTest);

        // ── 3. 收集精灵数据 ──
        var sprites = new List<SpriteInstance>();
        var spriteView = _scene.Scene.CreateView<TransformComponent, SpriteRendererComponent>();
        spriteView.Refresh();
        int totalSpriteEntities = 0;
        int skippedInvisible = 0;
        int skippedZeroTexture = 0;

        spriteView.ForEach(
            (IEntity entity, TransformComponent transform, SpriteRendererComponent sprite) =>
            {
                totalSpriteEntities++;

                // 跳过不可见的精灵
                if ((sprite.Flags & SpriteFlags.Visible) == 0)
                {
                    skippedInvisible++;
                    return;
                }

                var instance = new SpriteInstance();
                unsafe
                {
                    // System.Numerics 行主序 = HLSL 列主序（不需要转置，直接写入）
                    var worldMatrix = transform.WorldMatrix;
                    var matBytes = new byte[64];
                    MemoryMarshal.Write(matBytes.AsSpan(), in worldMatrix);
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

                instance.TextureHandle = RenderAssetManager.Instance.EnsureTextureLoaded(sprite.TextureAsset);
                instance.Layer = sprite.Layer;
                instance.SortOrder = sprite.SortOrder;
                instance.BlendMode = (uint)sprite.Blend;
                instance.Flags = 0;
                if ((sprite.Flags & SpriteFlags.FlipX) != 0) instance.Flags |= (uint)SpriteFlags.FlipX;
                if ((sprite.Flags & SpriteFlags.FlipY) != 0) instance.Flags |= (uint)SpriteFlags.FlipY;

                if (shouldLog)
                {
                    var texGuid = sprite.TextureAsset;
                    bool hasTexture = !texGuid.High.Equals(0) || !texGuid.Low.Equals(0);
                    if (!hasTexture) skippedZeroTexture++;

                    Console.WriteLine($"[ViewportService] Sprite[{totalSpriteEntities - 1}] entity={entity.Id}: " +
                        $"pos=({transform.Position.X:F2}, {transform.Position.Y:F2}, {transform.Position.Z:F2}), " +
                        $"scale=({transform.Scale.X:F2}, {transform.Scale.Y:F2}, {transform.Scale.Z:F2}), " +
                        $"color=({sprite.ColorR:F2}, {sprite.ColorG:F2}, {sprite.ColorB:F2}, {sprite.ColorA:F2}), " +
                        $"flags={sprite.Flags}, texGuid=0x{texGuid.High:X16}{texGuid.Low:X16}, " +
                        $"hasTexture={hasTexture}, layer={sprite.Layer}");
                }

                sprites.Add(instance);
            });

        if (shouldLog)
        {
            Console.WriteLine($"[ViewportService] 精灵统计: 总数={totalSpriteEntities}, " +
                $"不可见跳过={skippedInvisible}, 无纹理={skippedZeroTexture}, 最终提交={sprites.Count}");
        }

        if (sprites.Count > 0)
        {
            buffer.AddDrawSpriteBatch(CollectionsMarshal.AsSpan(sprites));
        }

        // ── 4. 收集 RmlUIDocument 组件（直接 Friflo Query，不依赖 IInspectorService） ──
        var rmlDocs = new List<RmlDocumentEntry>();
        if (AssetPathResolver != null)
        {
            var rmlView = _scene.Scene.CreateView<RmlUIDocumentComponent>();
            rmlView.Refresh();
            rmlView.ForEach((IEntity entity, RmlUIDocumentComponent doc) =>
            {
                // 通过注入的解析器获取 VFSService 路径
                var path = AssetPathResolver(doc.DocumentAsset);
                if (string.IsNullOrEmpty(path))
                    return;

                // 填充 RmlDocumentEntry（276 bytes）
                var entry = new RmlDocumentEntry();
                // 写入 UTF-8 路径到 fixed byte[256]
                var pathBytes = System.Text.Encoding.UTF8.GetBytes(path);
                int copyLen = Math.Min(pathBytes.Length, 255); // 留 1 字节 NUL
                unsafe
                {
                    fixed (byte* src = pathBytes)
                    {
                        byte* dst = entry.AssetPath;
                        for (int j = 0; j < copyLen; j++)
                            dst[j] = src[j];
                        dst[copyLen] = 0; // NUL 终结
                    }
                }
                entry.SortOrder = doc.SortOrder;
                entry.ViewTarget = (uint)doc.ViewTarget;
                entry.EntityHandle = (uint)entity.Id;
                entry.ViewportId = 0;
                rmlDocs.Add(entry);
            });
        }

        if (rmlDocs.Count > 0)
        {
            buffer.AddSetRmlDocuments(CollectionsMarshal.AsSpan(rmlDocs));
        }

        // ── 5. 构建并返回 ──
        var result = buffer.Build();
        if (shouldLog)
            Console.WriteLine($"[ViewportService] RenderCommands 构建完成: {result.Length} bytes, {buffer.CommandCount} commands");

        return result;
    }

}
