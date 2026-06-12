using Avalonia.Controls;
using Avalonia.Layout;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Avalonia Inspector 基类——组件检查器的抽象基类。
///
/// 与 ImGui 的 ComponentTypeInspector&lt;T&gt; 对应。
/// 每个组件类型对应一个 Inspector 实现。
///
/// 使用方式：
///   public class TransformInspector : AvaloniaInspectorBase
///   {
///       public override bool CanInspect(ulong typeId) => typeId == TransformTypeId;
///       public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
///       {
///           // 创建 Transform 编辑 UI
///       }
///   }
/// </summary>
public abstract class AvaloniaInspectorBase
{
    /// <summary>Inspector 显示名称。</summary>
    public abstract string DisplayName { get; }

    /// <summary>检查是否能检查指定类型的组件。</summary>
    public abstract bool CanInspect(ulong typeId);

    /// <summary>
    /// 创建组件检查器 UI。
    /// </summary>
    /// <param name="sceneHandle">场景句柄。</param>
    /// <param name="entityHandle">实体句柄。</param>
    /// <param name="typeId">组件类型 ID。</param>
    /// <returns>组件检查器控件。</returns>
    public abstract Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId);

    /// <summary>
    /// 创建可折叠的组件面板。
    /// </summary>
    /// <param name="title">面板标题。</param>
    /// <param name="content">面板内容。</param>
    /// <param name="isExpanded">是否展开。</param>
    /// <returns>可折叠面板控件。</returns>
    protected static Control CreateCollapsiblePanel(string title, Control content, bool isExpanded = true)
    {
        var panel = new DockPanel();

        // 头部（可点击展开/折叠）
        var header = new DockPanel
        {
            Background = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF3C3C3C")),
            Height = 28,
        };

        var expandIcon = new TextBlock
        {
            Text = isExpanded ? "▼" : "▶",
            Width = 20,
            FontSize = 10,
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Center,
        };
        Avalonia.Controls.DockPanel.SetDock(expandIcon, Avalonia.Controls.Dock.Left);
        header.Children.Add(expandIcon);

        var titleBlock = new TextBlock
        {
            Text = title,
            FontSize = 12,
            FontWeight = Avalonia.Media.FontWeight.Bold,
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(4, 0),
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FFCCCCCC")),
        };
        header.Children.Add(titleBlock);

        DockPanel.SetDock(header, Avalonia.Controls.Dock.Top);
        panel.Children.Add(header);

        // 内容区域
        content.IsVisible = isExpanded;
        panel.Children.Add(content);

        // 点击头部展开/折叠
        header.PointerPressed += (_, _) =>
        {
            content.IsVisible = !content.IsVisible;
            expandIcon.Text = content.IsVisible ? "▼" : "▶";
        };

        return panel;
    }

    /// <summary>
    /// 创建带标签的属性行。
    /// </summary>
    protected static Control CreatePropertyRow(string label, Control editor)
    {
        var panel = new DockPanel
        {
            Margin = new Avalonia.Thickness(4, 2),
        };

        var labelBlock = new TextBlock
        {
            Text = label,
            Width = 100,
            FontSize = 12,
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF999999")),
        };
        Avalonia.Controls.DockPanel.SetDock(labelBlock, Avalonia.Controls.Dock.Left);
        panel.Children.Add(labelBlock);

        panel.Children.Add(editor);

        return panel;
    }

    /// <summary>
    /// 创建数值输入框。
    /// </summary>
    protected static NumericUpDown CreateNumericInput(float value, float increment = 0.1f)
    {
        return new NumericUpDown
        {
            Value = (decimal)value,
            Increment = (decimal)increment,
            FormatString = "F3",
            Width = 100,
            Height = 24,
        };
    }

    /// <summary>
    /// 创建三轴数值输入（X/Y/Z）。
    /// </summary>
    protected static Control CreateVector3Input(string label, float x, float y, float z)
    {
        var panel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
        };

        var xInput = CreateNumericInput(x);
        var yInput = CreateNumericInput(y);
        var zInput = CreateNumericInput(z);

        // 轴标签
        var xLabel = new TextBlock
        {
            Text = "X",
            FontSize = 10,
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FFF44336")),
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
        };
        var yLabel = new TextBlock
        {
            Text = "Y",
            FontSize = 10,
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF4CAF50")),
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
        };
        var zLabel = new TextBlock
        {
            Text = "Z",
            FontSize = 10,
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF2196F3")),
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
        };

        panel.Children.Add(xLabel);
        panel.Children.Add(xInput);
        panel.Children.Add(yLabel);
        panel.Children.Add(yInput);
        panel.Children.Add(zLabel);
        panel.Children.Add(zInput);

        return CreatePropertyRow(label, panel);
    }
}
