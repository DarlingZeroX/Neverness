using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
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

        // ── 纹理（支持从 ContentBrowser 拖拽资产）──
        // 获取当前组件的纹理 GUID，用于显示初始纹理名称
        string? currentTextureName = GetCurrentTextureName(entityHandle);

        var textureArea = CreateAssetDropTarget(
            currentTextureName ?? "Drop\nTexture",
            assetPath => ApplyTextureAsset(assetPath, entityHandle));
        content.Children.Add(CreatePropertyRow("Texture", textureArea));

        // 颜色 RGBA（与 Transform 的 XYZ 同风格）
        content.Children.Add(CreateColorRow("Color", 255, 255, 255, 255));

        content.Children.Add(CreatePropertyRow("Layer", CreateNumericInput(0, 1)));
        content.Children.Add(CreatePropertyRow("Sort Order", CreateNumericInput(0, 1)));
        content.Children.Add(CreatePropertyRow("Blend Mode", CreateComboBox(new[] { "Alpha", "Additive", "Multiply" })));

        return CreateCollapsiblePanel("SpriteRenderer", content);
    }

    /// <summary>
    /// 拖拽纹理资产到 Inspector 后，更新 ECS 中 SpriteRendererComponent 的 TextureAsset 字段。
    /// </summary>
    private static void ApplyTextureAsset(string assetPath, ulong entityHandle)
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
    private static string? GetCurrentTextureName(ulong entityHandle)
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

    /// <summary>
    /// 通过 IInspectorService 获取实体（避免直接依赖 internal 的 SceneModuleImp）。
    /// </summary>
    private static Runtime.Scene.IEntity? GetEntityById(int entityId)
    {
        try
        {
            var context = EditorCoreModule.Context;
            if (context.TryGetService<IInspectorService>(out var inspectorService))
            {
                return inspectorService.GetEntityById(entityId);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SpriteRenderer] 获取实体失败: {ex.Message}");
        }
        return null;
    }
}
