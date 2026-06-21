using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 工具栏 Avalonia View——渲染常用操作按钮。
///
/// 与 ImGui 的 ImGuiToolbarRenderer 对应。
/// 从 ToolbarManager 读取按钮定义，渲染为 Avalonia 工具栏。
/// </summary>
public class ToolbarAvaloniaView : AvaloniaViewBase
{
    private StackPanel? _toolbarPanel;

    public ToolbarAvaloniaView() : base("Toolbar")
    {
    }

    public override Type ViewModelType => typeof(object); // 工具栏没有 ViewModel

    public override void Bind(object viewModel)
    {
        // 工具栏不绑定 ViewModel，直接从 ToolbarManager 读取
        CreateToolbar();
    }

    public override void Unbind()
    {
        _toolbarPanel = null;
    }

    /// <summary>获取工具栏控件。</summary>
    public StackPanel? GetToolbar() => _toolbarPanel;

    /// <summary>创建工具栏。</summary>
    private void CreateToolbar()
    {
        _toolbarPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Margin = new Avalonia.Thickness(0),
            Height = 40,
        };

        // 从 ToolbarManager 读取按钮定义
        // TODO: 实现工具栏注册表读取
        // 当前创建默认工具栏

        // 播放模式按钮
        AddToolButton("▶", "Play", () => ExecuteCommand("scene.play"));
        AddToolButton("⏸", "Pause", () => ExecuteCommand("scene.pause"));
        AddToolButton("⏹", "Stop", () => ExecuteCommand("scene.stop"));

        AddSeparator();

        // 变换工具
        AddToolButton("↔", "Translate", () => SetGizmoMode("Translate"));
        AddToolButton("↻", "Rotate", () => SetGizmoMode("Rotate"));
        AddToolButton("⤡", "Scale", () => SetGizmoMode("Scale"));

        AddSeparator();

        // 坐标系
        AddToolButton("🌍", "World", () => SetCoordinateSystem("World"));
        AddToolButton("📦", "Local", () => SetCoordinateSystem("Local"));

        AddSeparator();

        // 网格
        AddToolButton("▦", "Grid", () => ToggleGrid());

        AddSeparator();

        // 常用操作
        AddToolButton("💾", "Save", () => ExecuteCommand("file.save"));
        AddToolButton("↩", "Undo", () => ExecuteCommand("edit.undo"));
        AddToolButton("↪", "Redo", () => ExecuteCommand("edit.redo"));
    }

    /// <summary>添加工具栏按钮（用 Border+TextBlock 避免 Button 模板的额外空间）。</summary>
    private void AddToolButton(string icon, string tooltip, Action onClick)
    {
        var iconText = new TextBlock
        {
            Text = icon,
            FontSize = 20,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };

        var border = new Border
        {
            Width = 40,
            Height = 40,
            Background = Brushes.Transparent,
            Margin = new Avalonia.Thickness(0),
            Padding = new Avalonia.Thickness(0),
            Child = iconText,
            Cursor = new Cursor(StandardCursorType.Hand),
        };

        ToolTip.SetTip(border, tooltip);

        border.PointerPressed += (_, _) => onClick();

        border.PointerEntered += (_, _) =>
        {
            border.Background = new SolidColorBrush(Color.Parse("#FF3C3C3C"));
        };
        border.PointerExited += (_, _) =>
        {
            border.Background = Brushes.Transparent;
        };

        _toolbarPanel?.Children.Add(border);
    }

    /// <summary>添加分隔线。</summary>
    private void AddSeparator()
    {
        var separator = new Border
        {
            Width = 1,
            Height = 32,
            Background = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            Margin = new Avalonia.Thickness(6, 0),
            VerticalAlignment = VerticalAlignment.Center,
        };
        _toolbarPanel?.Children.Add(separator);
    }

    /// <summary>执行命令。</summary>
    private void ExecuteCommand(string commandId)
    {
        Console.WriteLine($"[Toolbar] 执行命令: {commandId}");
        // TODO: 通过 EditorCommandRegistry 执行命令
    }

    /// <summary>设置 Gizmo 模式。</summary>
    private void SetGizmoMode(string mode)
    {
        Console.WriteLine($"[Toolbar] Gizmo 模式: {mode}");
        // TODO: 设置 Gizmo 模式
    }

    /// <summary>设置坐标系。</summary>
    private void SetCoordinateSystem(string system)
    {
        Console.WriteLine($"[Toolbar] 坐标系: {system}");
        // TODO: 设置坐标系
    }

    /// <summary>切换网格显示。</summary>
    private void ToggleGrid()
    {
        Console.WriteLine("[Toolbar] 切换网格");
        // TODO: 切换网格显示
    }
}
