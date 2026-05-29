using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 通用资产拖拽框架——统一 ContentBrowser 拖拽源和 Inspector 拖拽目标的协议。
///
/// 设计原则：
/// 1. Payload 格式统一：128-bit GUID + 64-bit TypeId，共 24 字节
/// 2. 每种资产类型有独立的 payload 名称（如 "NN_TEXTURE"、"NN_AUDIO"）
/// 3. Inspector 通过 TryAcceptDragDrop 接收并验证资产类型
/// 4. ContentBrowser 通过 SetDragDropPayload 发起拖拽
///
/// 注意：此类不直接引用 AssetTypeId（避免 Framework → Assets 反向依赖）。
/// 调用方传入 ulong typeId 或 payloadName 字符串。
/// </summary>
public static class AssetDragDrop
{
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
            _ => Any,
        };
    }

    /// <summary>
    /// 发起资产拖拽源（ContentBrowser 使用）。
    /// 自动设置 payload，包含 GUID + TypeId。
    /// </summary>
    /// <param name="guid">资产 GUID。</param>
    /// <param name="typeId">资产类型 ID（使用 AssetTypeId 常量）。</param>
    /// <param name="displayName">显示名称（拖拽时预览文本）。</param>
    /// <returns>是否成功发起拖拽源。</returns>
    public static unsafe bool SetDragDropPayload(GUID guid, ulong typeId, string displayName)
    {
        if (guid.IsZero)
            return false;

        string payloadName = GetPayloadName(typeId);

        if (ImGui.BeginDragDropSource())
        {
            ulong* data = stackalloc ulong[3]; // GUID.High + GUID.Low + TypeId
            data[0] = guid.High;
            data[1] = guid.Low;
            data[2] = typeId;
            ImGui.SetDragDropPayload(payloadName, data, (nuint)(sizeof(ulong) * 3));
            ImGui.Text(displayName);
            ImGui.EndDragDropSource();
            return true;
        }

        return false;
    }

    /// <summary>
    /// 发起资产拖拽源（ContentBrowser 使用，通过 Importer 类型名称）。
    /// </summary>
    /// <param name="guid">资产 GUID。</param>
    /// <param name="importerType">Importer 类型名称（如 "TextureImporter"）。</param>
    /// <param name="displayName">显示名称。</param>
    /// <returns>是否成功发起拖拽源。</returns>
    public static unsafe bool SetDragDropPayload(GUID guid, string importerType, string displayName)
    {
        if (guid.IsZero)
            return false;

        string payloadName = GetPayloadNameByImporter(importerType);

        if (ImGui.BeginDragDropSource())
        {
            ulong* data = stackalloc ulong[3]; // GUID.High + GUID.Low + TypeId (0 = unknown)
            data[0] = guid.High;
            data[1] = guid.Low;
            data[2] = 0; // TypeId 未知，由 Inspector 根据 payloadName 判断
            ImGui.SetDragDropPayload(payloadName, data, (nuint)(sizeof(ulong) * 3));
            ImGui.Text(displayName);
            ImGui.EndDragDropSource();
            return true;
        }

        return false;
    }

    /// <summary>
    /// 尝试接受资产拖拽目标（Inspector 使用）。
    /// 检查 payload 名称是否匹配指定的资产类型。
    /// </summary>
    /// <param name="expectedTypeId">期望的资产类型 ID（使用 AssetTypeId 常量）。</param>
    /// <param name="guid">接收到的资产 GUID。</param>
    /// <param name="typeId">接收到的资产类型 ID（可能为 0）。</param>
    /// <returns>是否成功接收到匹配的资产。</returns>
    public static unsafe bool TryAcceptDragDrop(ulong expectedTypeId, out GUID guid, out ulong typeId)
    {
        guid = default;
        typeId = 0;

        string payloadName = GetPayloadName(expectedTypeId);
        return TryAcceptDragDrop(payloadName, out guid, out typeId);
    }

    /// <summary>
    /// 尝试接受资产拖拽目标（Inspector 使用）。
    /// 检查指定的 payload 名称。
    /// </summary>
    /// <param name="payloadName">要接受的 payload 名称（使用 AssetDragDrop.Texture 等常量）。</param>
    /// <param name="guid">接收到的资产 GUID。</param>
    /// <param name="typeId">接收到的资产类型 ID（可能为 0）。</param>
    /// <returns>是否成功接收到匹配的资产。</returns>
    public static unsafe bool TryAcceptDragDrop(string payloadName, out GUID guid, out ulong typeId)
    {
        guid = default;
        typeId = 0;

        var payload = ImGui.AcceptDragDropPayload(payloadName);
        if (payload.IsNull)
            return false;

        ulong* ptr = (ulong*)payload.Data;
        guid = new GUID(ptr[0], ptr[1]);
        typeId = ptr[2];

        return !guid.IsZero;
    }

    /// <summary>
    /// 尝试接受任意类型的资产拖拽目标。
    /// Inspector 可根据返回的 typeId 判断是否是自己需要的资产。
    /// </summary>
    /// <param name="guid">接收到的资产 GUID。</param>
    /// <param name="typeId">接收到的资产类型 ID。</param>
    /// <returns>是否成功接收到资产。</returns>
    public static unsafe bool TryAcceptAnyDragDrop(out GUID guid, out ulong typeId)
    {
        guid = default;
        typeId = 0;

        // 尝试接受所有已知类型的 payload
        string[] payloadNames = [Texture, Audio, Video, Mesh, Material, Any];

        foreach (var name in payloadNames)
        {
            var payload = ImGui.AcceptDragDropPayload(name);
            if (!payload.IsNull)
            {
                ulong* ptr = (ulong*)payload.Data;
                guid = new GUID(ptr[0], ptr[1]);
                typeId = ptr[2];
                return !guid.IsZero;
            }
        }

        return false;
    }

    /// <summary>
    /// 开始拖拽目标区域（Inspector 使用）。
    /// 返回 DragDropTargetScope，配合 using 使用。
    /// </summary>
    public static DragDropTargetScope BeginDragDropTarget()
    {
        return new DragDropTargetScope(ImGui.BeginDragDropTarget());
    }

    /// <summary>
    /// 拖拽目标作用域，自动调用 EndDragDropTarget。
    /// </summary>
    public struct DragDropTargetScope : IDisposable
    {
        private readonly bool _active;

        public DragDropTargetScope(bool active)
        {
            _active = active;
        }

        public bool IsActive => _active;

        public void Dispose()
        {
            if (_active)
                ImGui.EndDragDropTarget();
        }
    }
}
