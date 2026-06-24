using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets;
using Neverness.Editor.AvaloniaFrontend.PropertyEditor;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// SpriteRenderer 组件检查器——编辑纹理、颜色、UV 等。
///
/// 对应 ImGui 版本的 SpriteRendererInspector。
/// </summary>
public class SpriteRendererInspector : AvaloniaInspectorBase
{
    // SpriteRenderer 组件 TypeId（与 Native NNSpriteRendererComponentData 的 ComponentIdAttribute 一致）
    private const ulong SpriteRendererTypeId = 0x51387BA3968C343B;

    public override string DisplayName => "SpriteRenderer";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == SpriteRendererTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel { Spacing = 0 };

        // 获取当前组件数据
        var entity = GetEntityById((int)entityHandle);
        if (entity == null || !entity.IsValid || !entity.Has<SpriteRendererComponent>())
            return CreateCollapsiblePanel("SpriteRenderer", content);

        ref var sprite = ref entity.Get<SpriteRendererComponent>();

        // ── 纹理（支持从 ContentBrowser 拖拽资产）──
        string? currentTextureName = GetCurrentTextureName(entityHandle);
        var textureArea = AssetReferenceField.Create(
            currentTextureName ?? "Drop\nTexture",
            assetPath => ApplyTextureAsset(assetPath, entityHandle));
        content.Children.Add(PropertyRows.Create("Texture", textureArea));

        // ── 颜色 RGBA（0-255 显示，内部 0-1）──
        content.Children.Add(VectorFields.CreateColor("Color",
            sprite.ColorR * 255f, sprite.ColorG * 255f, sprite.ColorB * 255f, sprite.ColorA * 255f,
            (r, g, b, a) =>
            {
                if (!entity.IsValid || !entity.Has<SpriteRendererComponent>()) return;
                ref var s = ref entity.Get<SpriteRendererComponent>();
                s.ColorR = r / 255f;
                s.ColorG = g / 255f;
                s.ColorB = b / 255f;
                s.ColorA = a / 255f;
            }));

        // ── Layer ──
        content.Children.Add(PropertyRows.Create("Layer",
            NumericFields.CreateUInt(sprite.Layer, 1, value =>
            {
                if (!entity.IsValid || !entity.Has<SpriteRendererComponent>()) return;
                ref var s = ref entity.Get<SpriteRendererComponent>();
                s.Layer = value;
            })));

        // ── Sort Order ──
        content.Children.Add(PropertyRows.Create("Sort Order",
            NumericFields.CreateUInt(sprite.SortOrder, 1, value =>
            {
                if (!entity.IsValid || !entity.Has<SpriteRendererComponent>()) return;
                ref var s = ref entity.Get<SpriteRendererComponent>();
                s.SortOrder = value;
            })));

        // ── Blend Mode ──
        var blendModes = Enum.GetNames<BlendMode>();
        content.Children.Add(PropertyRows.Create("Blend Mode",
            NumericFields.CreateCombo(blendModes, (int)sprite.Blend, index =>
            {
                if (!entity.IsValid || !entity.Has<SpriteRendererComponent>()) return;
                ref var s = ref entity.Get<SpriteRendererComponent>();
                s.Blend = (BlendMode)index;
            })));

        return CreateCollapsiblePanel("SpriteRenderer", content);
    }

    /// <summary>
    /// 拖拽纹理资产到 Inspector 后，更新 ECS 中 SpriteRendererComponent 的 TextureAsset 字段。
    /// </summary>
    private void ApplyTextureAsset(string assetPath, ulong entityHandle)
    {
        // 1. 从路径查找资产 GUID
        var virtualPath = new NVirtualPath(assetPath);
        if (!EditorAssetDatabase.TryGetGuid(virtualPath, out var guid) || guid.IsZero)
        {
            Console.WriteLine($"[SpriteRenderer] 无法找到资产 GUID: {assetPath}");
            return;
        }

        // 2. 校验资产类型必须是纹理
        var meta = EditorAssetDatabase.TryGetMeta(virtualPath);
        if (meta == null || meta.AssetTypeId != AssetTypeId.Texture2D)
        {
            Console.WriteLine($"[SpriteRenderer] 资产不是纹理类型: {assetPath} (TypeId={meta?.AssetTypeId})");
            return;
        }

        // 3. 获取 ECS 实体和组件（通过公开的 IInspectorService 接口）
        var entity = GetEntityById((int)entityHandle);
        if (entity == null || !entity.IsValid) return;
        if (!entity.Has<SpriteRendererComponent>()) return;

        // 4. 更新纹理资产 GUID（ref 返回直接写入 ECS）
        ref var sprite = ref entity.Get<SpriteRendererComponent>();
        sprite.TextureAsset = new NNGuid { High = guid.High, Low = guid.Low };

        Console.WriteLine($"[SpriteRenderer] 纹理已更新: {assetPath} → GUID=0x{guid.ToHexString()}");
    }

    /// <summary>
    /// 获取当前 SpriteRenderer 组件的纹理资产名称（用于初始化显示）。
    /// </summary>
    private string? GetCurrentTextureName(ulong entityHandle)
    {
        try
        {
            var entity = GetEntityById((int)entityHandle);
            if (entity == null || !entity.IsValid) return null;
            if (!entity.Has<SpriteRendererComponent>()) return null;

            ref var sprite = ref entity.Get<SpriteRendererComponent>();
            var textureGuid = new GUID(sprite.TextureAsset.High, sprite.TextureAsset.Low);
            if (textureGuid.IsZero) return null;

            if (EditorAssetDatabase.TryGetPath(textureGuid, out var path))
            {
                return System.IO.Path.GetFileNameWithoutExtension(path.FullPath);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SpriteRenderer] 获取当前纹理名称失败: {ex.Message}");
        }
        return null;
    }

}
