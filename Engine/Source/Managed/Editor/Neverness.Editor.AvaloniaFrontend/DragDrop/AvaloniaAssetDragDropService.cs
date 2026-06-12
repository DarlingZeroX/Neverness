using System.Numerics;
using Avalonia.Controls;
using Avalonia.Input;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.DragDrop;

/// <summary>
/// Avalonia 资产拖拽服务实现——封装 Avalonia 拖放 API。
///
/// 与 ImGui 的 ImGuiAssetDragDropService 对应。
/// 实现 IAssetDragDropService 接口，使 Core 模块不依赖具体 UI 框架。
///
/// TODO: 完整实现需要适配 Avalonia 11.3.x 的 DragDrop API
/// 当前为最小化实现，确保编译通过。
/// </summary>
public class AvaloniaAssetDragDropService : IAssetDragDropService
{
    // 拖拽数据格式常量
    private const string AssetPayloadFormat = "NevernessAsset";

    // 当前拖拽状态
    private GUID _currentDragGuid;
    private ulong _currentDragTypeId;
    private string _currentDragDisplayName = "";
    private bool _isDragging;

    /// <summary>发起资产拖拽源（通过 TypeId）。</summary>
    public bool SetDragDropPayload(GUID guid, ulong typeId, string displayName)
    {
        _currentDragGuid = guid;
        _currentDragTypeId = typeId;
        _currentDragDisplayName = displayName;
        _isDragging = true;

        Console.WriteLine($"[AvaloniaAssetDragDrop] 开始拖拽: {displayName} (GUID={guid}, TypeId={typeId})");
        return true;
    }

    /// <summary>发起资产拖拽源（通过 Importer 类型名称）。</summary>
    public bool SetDragDropPayload(GUID guid, string importerType, string displayName)
    {
        // 将 importerType 转换为 typeId（简化实现）
        // TODO: 使用实际的类型映射
        return SetDragDropPayload(guid, 0, displayName);
    }

    /// <summary>尝试接受指定类型的资产拖拽。</summary>
    public bool TryAcceptDragDrop(ulong expectedTypeId, out GUID guid, out ulong typeId)
    {
        if (_isDragging && _currentDragTypeId == expectedTypeId)
        {
            guid = _currentDragGuid;
            typeId = _currentDragTypeId;
            return true;
        }

        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>尝试接受指定 payload 名称的资产拖拽。</summary>
    public bool TryAcceptDragDrop(string payloadName, out GUID guid, out ulong typeId)
    {
        if (_isDragging && _currentDragDisplayName == payloadName)
        {
            guid = _currentDragGuid;
            typeId = _currentDragTypeId;
            return true;
        }

        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>尝试接受任意类型的资产拖拽。</summary>
    public bool TryAcceptAnyDragDrop(out GUID guid, out ulong typeId)
    {
        if (_isDragging)
        {
            guid = _currentDragGuid;
            typeId = _currentDragTypeId;
            return true;
        }

        guid = default;
        typeId = 0;
        return false;
    }

    /// <summary>开始拖拽目标区域（配合 using 自动结束）。</summary>
    public DragDropTargetScope BeginDragDropTarget()
    {
        // 在 Avalonia 中，拖拽目标通过事件处理
        // 此处返回一个作用域，用于自动清理
        return new DragDropTargetScope(true, () =>
        {
            // 清理拖拽状态
            _isDragging = false;
        });
    }

    /// <summary>
    /// 处理 Avalonia 拖拽事件（从外部控件调用）。
    /// TODO: 完整实现需要适配 Avalonia 11.3.x 的 DragDrop API
    /// </summary>
    public void HandleDragOver(object? sender, object e)
    {
        // TODO: 适配 Avalonia 11.3.x DragEventArgs
        Console.WriteLine("[AvaloniaAssetDragDrop] DragOver");
    }

    /// <summary>
    /// 处理 Avalonia 放置事件（从外部控件调用）。
    /// TODO: 完整实现需要适配 Avalonia 11.3.x 的 DragDrop API
    /// </summary>
    public void HandleDrop(object? sender, object e)
    {
        // TODO: 适配 Avalonia 11.3.x DragEventArgs
        Console.WriteLine("[AvaloniaAssetDragDrop] Drop");

        // 清理拖拽状态
        _isDragging = false;
    }
}
