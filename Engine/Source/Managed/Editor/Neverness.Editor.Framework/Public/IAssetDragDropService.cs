using System.Numerics;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 资产拖拽服务接口——由 ImGuiFrontend 实现。
///
/// 封装 ImGui 拖拽源/目标操作，使调用方无需直接引用 ImGui。
/// 使用方式：
///   // 发起拖拽
///   _dragDropService.SetDragDropPayload(guid, typeId, displayName);
///
///   // 接受拖拽
///   using var target = _dragDropService.BeginDragDropTarget();
///   if (target.IsActive && _dragDropService.TryAcceptDragDrop(expectedTypeId, out var guid, out var typeId))
///   { ... }
/// </summary>
public interface IAssetDragDropService
{
    /// <summary>发起资产拖拽源（通过 TypeId）。</summary>
    bool SetDragDropPayload(GUID guid, ulong typeId, string displayName);

    /// <summary>发起资产拖拽源（通过 Importer 类型名称）。</summary>
    bool SetDragDropPayload(GUID guid, string importerType, string displayName);

    /// <summary>尝试接受指定类型的资产拖拽。</summary>
    bool TryAcceptDragDrop(ulong expectedTypeId, out GUID guid, out ulong typeId);

    /// <summary>尝试接受指定 payload 名称的资产拖拽。</summary>
    bool TryAcceptDragDrop(string payloadName, out GUID guid, out ulong typeId);

    /// <summary>尝试接受任意类型的资产拖拽。</summary>
    bool TryAcceptAnyDragDrop(out GUID guid, out ulong typeId);

    /// <summary>开始拖拽目标区域（配合 using 自动结束）。</summary>
    DragDropTargetScope BeginDragDropTarget();
}

/// <summary>
/// 拖拽目标作用域——配合 using 使用，自动调用 EndDragDropTarget。
/// </summary>
public struct DragDropTargetScope : IDisposable
{
    private readonly bool _active;
    private readonly Action? _endAction;

    public DragDropTargetScope(bool active, Action? endAction)
    {
        _active = active;
        _endAction = endAction;
    }

    public bool IsActive => _active;

    public void Dispose()
    {
        if (_active)
            _endAction?.Invoke();
    }
}
