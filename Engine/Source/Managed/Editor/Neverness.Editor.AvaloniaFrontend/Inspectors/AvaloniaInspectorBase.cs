using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.Styling;
using Neverness.Editor.Core.Public;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Avalonia Inspector 基类——组件检查器的抽象基类。
/// 只定义 Inspector 接口和 Inspector 专属的 UI 方法。
/// 通用属性编辑控件请使用 PropertyEditor 命名空间下的类。
/// </summary>
public abstract class AvaloniaInspectorBase
{
    public abstract string DisplayName { get; }
    public abstract bool CanInspect(ulong typeId);
    public abstract Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId);

    // ── 实体访问（所有子类共用）──

    /// <summary>通过 IInspectorService 获取实体，日志前缀自动使用子类名。</summary>
    protected Runtime.Scene.IEntity? GetEntityById(int entityId)
    {
        try
        {
            var context = EditorCoreModule.Context;
            if (context.TryGetService<IInspectorService>(out var inspectorService))
            {
                return inspectorService.GetEntityById(entityId);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[{GetType().Name}] 获取实体失败: {ex.Message}");
        }
        return null;
    }

    // ── 可折叠组件面板（Inspector 专属）──

    /// <summary>创建可折叠的组件面板。</summary>
    protected static Control CreateCollapsiblePanel(string title, Control content, bool isExpanded = true)
    {
        var root = new DockPanel { Margin = new Thickness(0, 0, 0, 2) };

        // 头部
        var header = new DockPanel
        {
            Background = EditorTheme.HeaderBackground,
            Height = 26,
            Cursor = new Cursor(StandardCursorType.Hand),
        };

        var expandIcon = new TextBlock
        {
            Text = isExpanded ? "▼" : "▶",
            Width = 20,
            FontSize = 9,
            VerticalAlignment = VerticalAlignment.Center,
            HorizontalAlignment = HorizontalAlignment.Center,
            Foreground = EditorTheme.TextSecondary,
        };
        DockPanel.SetDock(expandIcon, Avalonia.Controls.Dock.Left);
        header.Children.Add(expandIcon);

        var titleBlock = new TextBlock
        {
            Text = title,
            FontSize = 12,
            FontWeight = FontWeight.Bold,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(4, 0),
            Foreground = EditorTheme.TextPrimary,
        };
        header.Children.Add(titleBlock);

        DockPanel.SetDock(header, Avalonia.Controls.Dock.Top);
        root.Children.Add(header);

        // 内容区域
        content.IsVisible = isExpanded;
        root.Children.Add(content);

        // 点击展开/折叠
        header.PointerPressed += (_, _) =>
        {
            content.IsVisible = !content.IsVisible;
            expandIcon.Text = content.IsVisible ? "▼" : "▶";
        };

        return root;
    }
}
