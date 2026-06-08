using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.ImGuiFrontend.DragDrop;

/// <summary>
/// ImGui 资产拖拽服务——实现 IAssetDragDropService 接口。
/// 从 AssetDragDrop 拆分出的 ImGui 拖拽操作。
/// </summary>
public sealed class ImGuiAssetDragDropService : IAssetDragDropService
{
    /// <summary>发起资产拖拽源（通过 TypeId）。</summary>
    public unsafe bool SetDragDropPayload(GUID guid, ulong typeId, string displayName)
    {
        if (guid.IsZero)
            return false;

        string payloadName = AssetDragDrop.GetPayloadName(typeId);

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

    /// <summary>发起资产拖拽源（通过 Importer 类型名称）。</summary>
    public unsafe bool SetDragDropPayload(GUID guid, string importerType, string displayName)
    {
        if (guid.IsZero)
            return false;

        string payloadName = AssetDragDrop.GetPayloadNameByImporter(importerType);

        if (ImGui.BeginDragDropSource())
        {
            ulong* data = stackalloc ulong[3];
            data[0] = guid.High;
            data[1] = guid.Low;
            data[2] = 0;
            ImGui.SetDragDropPayload(payloadName, data, (nuint)(sizeof(ulong) * 3));
            ImGui.Text(displayName);
            ImGui.EndDragDropSource();
            return true;
        }

        return false;
    }

    /// <summary>尝试接受指定类型的资产拖拽。</summary>
    public unsafe bool TryAcceptDragDrop(ulong expectedTypeId, out GUID guid, out ulong typeId)
    {
        guid = default;
        typeId = 0;

        string payloadName = AssetDragDrop.GetPayloadName(expectedTypeId);
        return TryAcceptDragDrop(payloadName, out guid, out typeId);
    }

    /// <summary>尝试接受指定 payload 名称的资产拖拽。</summary>
    public unsafe bool TryAcceptDragDrop(string payloadName, out GUID guid, out ulong typeId)
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

    /// <summary>尝试接受任意类型的资产拖拽。</summary>
    public unsafe bool TryAcceptAnyDragDrop(out GUID guid, out ulong typeId)
    {
        guid = default;
        typeId = 0;

        string[] payloadNames = [
            AssetDragDrop.Texture,
            AssetDragDrop.Audio,
            AssetDragDrop.Video,
            AssetDragDrop.Mesh,
            AssetDragDrop.Material,
            AssetDragDrop.Script,
            AssetDragDrop.Any
        ];

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

    /// <summary>开始拖拽目标区域。</summary>
    public DragDropTargetScope BeginDragDropTarget()
    {
        bool active = ImGui.BeginDragDropTarget();
        return new DragDropTargetScope(active, active ? ImGui.EndDragDropTarget : null);
    }
}
