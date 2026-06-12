using Neverness.Editor.Framework.Public.Services;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// Avalonia 窗口服务实现——管理独立窗口和对话框。
/// </summary>
public class AvaloniaWindowService : IWindowService
{
    public IEditorWindow CreateWindow(string title, int width, int height)
    {
        // TODO: Phase 6 实现 - 创建 Avalonia 独立窗口
        Console.WriteLine($"[AvaloniaWindowService] 创建窗口: {title} ({width}x{height})");
        return new AvaloniaEditorWindow(title, width, height);
    }

    public bool ShowConfirmDialog(string title, string message)
    {
        // TODO: Phase 6 实现 - 使用 Avalonia 对话框
        Console.WriteLine($"[AvaloniaWindowService] 确认对话框: {title} - {message}");
        return false;
    }

    public string? ShowInputDialog(string title, string message, string defaultValue = "")
    {
        // TODO: Phase 6 实现
        Console.WriteLine($"[AvaloniaWindowService] 输入对话框: {title} - {message}");
        return null;
    }

    public int ShowChoiceDialog(string title, string message, string[] options)
    {
        // TODO: Phase 6 实现
        Console.WriteLine($"[AvaloniaWindowService] 选择对话框: {title} - {message}");
        return -1;
    }

    public string? ShowOpenFileDialog(string title, string filter)
    {
        // TODO: Phase 6 实现
        Console.WriteLine($"[AvaloniaWindowService] 打开文件: {title}");
        return null;
    }

    public string? ShowSaveFileDialog(string title, string filter, string defaultName = "")
    {
        // TODO: Phase 6 实现
        Console.WriteLine($"[AvaloniaWindowService] 保存文件: {title}");
        return null;
    }

    public string? ShowFolderDialog(string title)
    {
        // TODO: Phase 6 实现
        Console.WriteLine($"[AvaloniaWindowService] 选择目录: {title}");
        return null;
    }
}

/// <summary>
/// Avalonia 编辑器窗口实现（占位）。
/// </summary>
internal class AvaloniaEditorWindow : IEditorWindow
{
    public string Title { get; set; }
    public bool IsVisible { get; private set; }

    public AvaloniaEditorWindow(string title, int width, int height)
    {
        Title = title;
    }

    public void Show() => IsVisible = true;
    public void Hide() => IsVisible = false;
    public void Close() => IsVisible = false;
    public void SetContent(object content) { }
    public void Dispose() { }
}
