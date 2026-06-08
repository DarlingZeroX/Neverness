using Neverness.Runtime.Assets;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 通用资产拖拽框架——定义 payload 名称常量和类型映射。
///
/// 设计原则：
/// 1. Payload 格式统一：128-bit GUID + 64-bit TypeId，共 24 字节
/// 2. 每种资产类型有独立的 payload 名称（如 "NN_TEXTURE"、"NN_AUDIO"）
/// 3. 此类只定义常量和映射方法（UI 无关）
/// 4. 实际的 ImGui 拖拽操作由 <see cref="IAssetDragDropService"/> 实现
///
/// 注意：此类不直接引用 AssetTypeId（避免 Framework → Assets 反向依赖）。
/// 调用方传入 ulong typeId 或 payloadName 字符串。
/// </summary>
public static class AssetDragDrop
{
    /// <summary>
    /// 拖拽服务实例——由 ImGuiFrontend 在启动时注入。
    /// 旧代码通过此类的方法间接调用服务，无需直接引用 ImGui。
    /// </summary>
    public static IAssetDragDropService? Service { get; set; }

    // ── Payload 名称常量 ──
    // 使用 "NN_" 前缀避免与 ImGui 内置或其他插件冲突

    /// <summary>纹理资产拖拽 payload。</summary>
    public const string Texture = "NN_TEXTURE";

    /// <summary>音频资产拖拽 payload。</summary>
    public const string Audio = "NN_AUDIO";

    /// <summary>视频资产拖拽 payload。</summary>
    public const string Video = "NN_VIDEO";

    /// <summary>网格资产拖拽 payload。</summary>
    public const string Mesh = "NN_MESH";

    /// <summary>材质资产拖拽 payload。</summary>
    public const string Material = "NN_MATERIAL";

    /// <summary>通用资产拖拽 payload（接受任意类型，Inspector 需自行验证 TypeId）。</summary>
    public const string Any = "NN_ASSET";

    /// <summary>C# 脚本资产拖拽 payload。</summary>
    public const string Script = "NN_SCRIPT";

    // ── 预定义 TypeId 常量（与 AssetTypeId / NNAssetTypes.h 对齐） ──
    // Framework 层不能引用 Assets 层，故在此重复定义。

    /// <summary>纹理资产 TypeId（= AssetTypeId.Texture2D）。</summary>
    public const ulong TypeIdTexture2D = 1;

    /// <summary>音频资产 TypeId（= AssetTypeId.AudioClip）。</summary>
    public const ulong TypeIdAudioClip = 3;

    /// <summary>视频资产 TypeId（= AssetTypeId.VideoClip）。</summary>
    public const ulong TypeIdVideoClip = 10;

    /// <summary>网格资产 TypeId（= AssetTypeId.Mesh）。</summary>
    public const ulong TypeIdMesh = 2;

    /// <summary>材质资产 TypeId（= AssetTypeId.Material）。</summary>
    public const ulong TypeIdMaterial = 4;

    /// <summary>C# 脚本资产 TypeId（= AssetTypeId.CSharpScript）。</summary>
    public const ulong TypeIdCSharpScript = 11;

    /// <summary>
    /// 根据资产类型 ID 获取对应的 payload 名称。
    /// </summary>
    public static string GetPayloadName(ulong typeId)
    {
        return typeId switch
        {
            TypeIdTexture2D => Texture,
            TypeIdAudioClip => Audio,
            TypeIdVideoClip => Video,
            TypeIdMesh => Mesh,
            TypeIdMaterial => Material,
            TypeIdCSharpScript => Script,
            _ => Any,
        };
    }

    /// <summary>
    /// 根据 Importer 类型名称获取对应的 payload 名称。
    /// 用于 ContentBrowser 中根据 file.AssetType 判断。
    /// </summary>
    public static string GetPayloadNameByImporter(string importerType)
    {
        return importerType switch
        {
            "TextureImporter" => Texture,
            "AudioImporter" => Audio,
            "VideoImporter" => Video,
            "MeshImporter" => Mesh,
            "MaterialImporter" => Material,
            "ScriptAssetImporter" => Script,
            _ => Any,
        };
    }

    // ── 拖拽操作委托（兼容旧代码，委托给 IAssetDragDropService） ──

    /// <summary>发起资产拖拽源（委托给 Service）。</summary>
    public static bool SetDragDropPayload(GUID guid, ulong typeId, string displayName)
    {
        return Service != null && Service.SetDragDropPayload(guid, typeId, displayName);
    }

    /// <summary>发起资产拖拽源（委托给 Service）。</summary>
    public static bool SetDragDropPayload(GUID guid, string importerType, string displayName)
    {
        return Service != null && Service.SetDragDropPayload(guid, importerType, displayName);
    }

    /// <summary>尝试接受资产拖拽（委托给 Service）。</summary>
    public static bool TryAcceptDragDrop(ulong expectedTypeId, out GUID guid, out ulong typeId)
    {
        if (Service != null)
            return Service.TryAcceptDragDrop(expectedTypeId, out guid, out typeId);
        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>尝试接受资产拖拽（委托给 Service）。</summary>
    public static bool TryAcceptDragDrop(string payloadName, out GUID guid, out ulong typeId)
    {
        if (Service != null)
            return Service.TryAcceptDragDrop(payloadName, out guid, out typeId);
        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>尝试接受任意类型的资产拖拽（委托给 Service）。</summary>
    public static bool TryAcceptAnyDragDrop(out GUID guid, out ulong typeId)
    {
        if (Service != null)
            return Service.TryAcceptAnyDragDrop(out guid, out typeId);
        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>开始拖拽目标区域（委托给 Service）。</summary>
    public static DragDropTargetScope BeginDragDropTarget()
    {
        return Service?.BeginDragDropTarget() ?? new DragDropTargetScope(false, null);
    }
}
