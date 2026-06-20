using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Platform.Storage;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.DragDrop;

/// <summary>
/// Avalonia 拖放处理器——处理控件级别的拖放事件。
///
/// 支持两种拖拽：
/// 1. 内部资产拖拽（Content Browser → Inspector）
/// 2. 外部文件拖拽（Windows 资源管理器 → Content Browser）
///
/// 使用方式：
///   var handler = new AvaloniaDropHandler();
///   handler.Attach(myControl);
///   handler.FilesDropped += files => HandleFiles(files);
/// </summary>
public class AvaloniaDropHandler
{
    private readonly List<Control> _attachedControls = new();

    /// <summary>外部文件拖入事件（文件系统路径数组）。</summary>
    public event Action<string[]>? FilesDropped;

    /// <summary>内部资产拖拽事件（GUID, TypeId）。</summary>
    public event Action<GUID, ulong>? AssetDropped;

    /// <summary>拖拽进入事件（用于触发高亮效果）。</summary>
    public event Action? DragEnter;

    /// <summary>拖拽离开事件（用于取消高亮效果）。</summary>
    public event Action? DragLeave;

    /// <summary>附加到控件，启用拖放支持。</summary>
    /// <param name="control">目标控件（必须有 Background 才能接收拖拽事件）。</param>
    /// <param name="eventHost">事件注册宿主（父容器），为 null 则注册在 control 上。</param>
    public void Attach(Control control, Control? eventHost = null)
    {
        if (_attachedControls.Contains(control))
            return;

        _attachedControls.Add(control);

        // 设置 AllowDrop（必须在目标控件上）
        Avalonia.Input.DragDrop.SetAllowDrop(control, true);

        // 事件注册在宿主上（参照官方示例：AddHandler 注册在父容器）
        // 如果没有指定宿主，则注册在目标控件上
        var host = eventHost ?? control;
        host.AddHandler(Avalonia.Input.DragDrop.DragEnterEvent, OnDragEnter, RoutingStrategies.Bubble);
        host.AddHandler(Avalonia.Input.DragDrop.DragOverEvent, OnDragOver, RoutingStrategies.Bubble);
        host.AddHandler(Avalonia.Input.DragDrop.DragLeaveEvent, OnDragLeave, RoutingStrategies.Bubble);
        host.AddHandler(Avalonia.Input.DragDrop.DropEvent, OnDrop, RoutingStrategies.Bubble);

        Console.WriteLine($"[AvaloniaDropHandler] 已附加: 目标={control.GetType().Name}, 宿主={host.GetType().Name}, AllowDrop={Avalonia.Input.DragDrop.GetAllowDrop(control)}");
    }

    /// <summary>从控件分离，禁用拖放支持。</summary>
    public void Detach(Control control)
    {
        if (!_attachedControls.Contains(control))
            return;

        _attachedControls.Remove(control);

        // 取消订阅
        Avalonia.Input.DragDrop.SetAllowDrop(control, false);
        // 注意：Avalonia 目前没有 RemoveHandler API，依赖控件销毁自动清理
    }

    /// <summary>分离所有控件。</summary>
    public void DetachAll()
    {
        foreach (var control in _attachedControls.ToList())
        {
            Detach(control);
        }
    }

    // ── 事件处理 ──

    private void OnDragEnter(object? sender, DragEventArgs e)
    {
        Console.WriteLine("[AvaloniaDropHandler] DragEnter 触发");

        // 检查是否包含文件
        if (HasFiles(e))
        {
            Console.WriteLine("[AvaloniaDropHandler] 检测到文件，设置 Copy 效果");
            e.DragEffects = DragDropEffects.Copy;
            DragEnter?.Invoke();
        }
        else
        {
            Console.WriteLine("[AvaloniaDropHandler] 未检测到文件，设置 None 效果");
            e.DragEffects = DragDropEffects.None;
        }
    }

    private void OnDragOver(object? sender, DragEventArgs e)
    {
        var hasFiles = HasFiles(e);
        e.DragEffects = hasFiles ? DragDropEffects.Copy : DragDropEffects.None;
    }

    private void OnDragLeave(object? sender, DragEventArgs e)
    {
        Console.WriteLine("[AvaloniaDropHandler] DragLeave 触发");
        DragLeave?.Invoke();
    }

    private void OnDrop(object? sender, DragEventArgs e)
    {
        Console.WriteLine("[AvaloniaDropHandler] Drop 触发");

        // 取消高亮
        DragLeave?.Invoke();

        if (!HasFiles(e))
        {
            Console.WriteLine("[AvaloniaDropHandler] Drop 数据不包含文件");
            return;
        }

        var files = GetFiles(e);
        Console.WriteLine($"[AvaloniaDropHandler] 获取到 {files.Length} 个文件");

        if (files.Length > 0)
        {
            FilesDropped?.Invoke(files);
        }
    }

    // ── 辅助方法 ──

    /// <summary>检查拖拽数据是否包含文件。</summary>
    private static bool HasFiles(DragEventArgs e)
    {
        try
        {
            var data = e.DataTransfer;
            if (data == null)
            {
                Console.WriteLine("[AvaloniaDropHandler] HasFiles: DataTransfer 为 null");
                return false;
            }

            Console.WriteLine($"[AvaloniaDropHandler] HasFiles: DataTransfer.Formats 数量 = {data.Formats.Count}");

            // 打印所有格式
            foreach (var format in data.Formats)
            {
                Console.WriteLine($"[AvaloniaDropHandler] HasFiles: 格式 = {format.Identifier}, Kind = {format.Kind}");
            }

            // 检查是否包含 File 格式
            var hasFileFormat = data.Contains(DataFormat.File);
            Console.WriteLine($"[AvaloniaDropHandler] HasFiles: 包含 File 格式 = {hasFileFormat}");

            return hasFileFormat;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AvaloniaDropHandler] HasFiles 异常: {ex.Message}");
            return false;
        }
    }

    /// <summary>提取拖拽的文件路径列表。</summary>
    private static string[] GetFiles(DragEventArgs e)
    {
        try
        {
            var data = e.DataTransfer;
            if (data == null)
                return Array.Empty<string>();

            // 使用 Avalonia 12.0 的 TryGetFiles 扩展方法
            var files = data.TryGetFiles();
            if (files == null || files.Length == 0)
            {
                Console.WriteLine("[AvaloniaDropHandler] GetFiles: TryGetFiles 返回空");
                return Array.Empty<string>();
            }

            Console.WriteLine($"[AvaloniaDropHandler] GetFiles: TryGetFiles 返回 {files.Length} 个文件");

            var result = new List<string>();
            foreach (var file in files)
            {
                var path = file.TryGetLocalPath();
                Console.WriteLine($"[AvaloniaDropHandler] GetFiles: 文件路径 = {path ?? "null"}");

                if (!string.IsNullOrEmpty(path))
                {
                    result.Add(path);
                }
            }

            Console.WriteLine($"[AvaloniaDropHandler] GetFiles: 返回 {result.Count} 个文件");
            return result.ToArray();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AvaloniaDropHandler] 获取文件失败: {ex.Message}");
            return Array.Empty<string>();
        }
    }
}
