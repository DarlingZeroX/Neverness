using Avalonia.Controls;
using Avalonia.Input;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.DragDrop;

/// <summary>
/// Avalonia 拖放处理器——处理控件级别的拖放事件。
///
/// 使用方式：
///   var handler = new AvaloniaDropHandler(assetDragDropService);
///   handler.Attach(myControl);
///
/// TODO: 完整实现需要适配 Avalonia 11.3.x 的 DragDrop API
/// 当前为最小化实现，确保编译通过。
/// </summary>
public class AvaloniaDropHandler
{
    private readonly AvaloniaAssetDragDropService _dragDropService;
    private readonly List<Control> _attachedControls = new();

    /// <summary>文件拖入事件。</summary>
    public event Action<string[]>? FilesDropped;

    /// <summary>资产拖拽事件。</summary>
    public event Action<GUID, ulong>? AssetDropped;

    public AvaloniaDropHandler(AvaloniaAssetDragDropService dragDropService)
    {
        _dragDropService = dragDropService;
    }

    /// <summary>附加到控件，启用拖放支持。</summary>
    public void Attach(Control control)
    {
        if (_attachedControls.Contains(control))
            return;

        _attachedControls.Add(control);

        // TODO: 订阅 DragDrop 事件（需要适配 Avalonia 11.3.x API）
        Console.WriteLine($"[AvaloniaDropHandler] 附加到控件: {control.GetType().Name}");
    }

    /// <summary>从控件分离，禁用拖放支持。</summary>
    public void Detach(Control control)
    {
        if (!_attachedControls.Contains(control))
            return;

        _attachedControls.Remove(control);

        // TODO: 取消订阅 DragDrop 事件
        Console.WriteLine($"[AvaloniaDropHandler] 从控件分离: {control.GetType().Name}");
    }

    /// <summary>分离所有控件。</summary>
    public void DetachAll()
    {
        foreach (var control in _attachedControls.ToList())
        {
            Detach(control);
        }
    }
}
