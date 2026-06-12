using Dock.Model.Controls;

namespace Neverness.Editor.AvaloniaFrontend.Dock;

/// <summary>
/// 编辑器 Dock 布局管理——保存/恢复布局。
/// </summary>
public class EditorDockLayout
{
    private readonly EditorDockFactory _factory;

    public EditorDockLayout(EditorDockFactory factory)
    {
        _factory = factory;
    }

    /// <summary>保存当前布局到文件。</summary>
    public bool SaveLayout(IRootDock layout, string? filePath = null)
    {
        try
        {
            Console.WriteLine("[EditorDockLayout] 布局保存（TODO）");
            return true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[EditorDockLayout] 保存失败: {ex.Message}");
            return false;
        }
    }

    /// <summary>从文件恢复布局。</summary>
    public IRootDock? LoadLayout(string? filePath = null)
    {
        try
        {
            Console.WriteLine("[EditorDockLayout] 布局恢复（TODO）");
            return null;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[EditorDockLayout] 恢复失败: {ex.Message}");
            return null;
        }
    }

    /// <summary>获取或创建默认布局。</summary>
    public IRootDock GetOrCreateLayout()
    {
        var layout = LoadLayout();
        if (layout != null)
            return layout;

        Console.WriteLine("[EditorDockLayout] 创建默认布局");
        return _factory.CreateDefaultLayout();
    }
}
