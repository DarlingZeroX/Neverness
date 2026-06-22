using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.ContentBrowser;
using Neverness.Editor.AvaloniaFrontend.Styling;

namespace Neverness.Editor.AvaloniaFrontend.PropertyEditor;

/// <summary>
/// 资产引用字段——可接收资产拖拽的引用控件。
/// 用于纹理、Mesh、材质等资产引用场景。
/// </summary>
public static class AssetReferenceField
{
    /// <summary>创建可接收资产拖拽的引用字段。</summary>
    /// <param name="placeholderText">未拖入资产时显示的占位文字（如 "Drop Texture"）。</param>
    /// <param name="onAssetDropped">拖入资产后的回调（参数为资产路径）。</param>
    /// <param name="width">控件宽度，默认 80。</param>
    /// <param name="height">控件高度，默认 80。</param>
    public static Control Create(
        string placeholderText,
        Action<string>? onAssetDropped,
        double width = 80,
        double height = 80)
    {
        // 拖拽高亮边框颜色
        var highlightBrush = new SolidColorBrush(Color.FromArgb(0xFF, 0x21, 0x96, 0xF3));

        // 资产名称显示
        var assetLabel = new TextBlock
        {
            Text = placeholderText,
            FontSize = 10,
            Foreground = EditorTheme.TextDisabled,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
            TextAlignment = TextAlignment.Center,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxLines = 2,
            Margin = new Thickness(4),
        };

        var dropArea = new Border
        {
            Width = width,
            Height = height,
            Background = EditorTheme.InputBackground,
            BorderBrush = EditorTheme.Border,
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(4),
            Child = assetLabel,
            Cursor = new Cursor(StandardCursorType.Hand),
        };

        // 检查拖拽数据是否包含资产格式
        static bool HasAssetData(DragEventArgs e)
        {
            return e.DataTransfer.Contains(AssetDragFormats.SystemPath) ||
                   e.DataTransfer.Contains(AssetDragFormats.VirtualPath);
        }

        // 拖拽进入 → 高亮
        Avalonia.Input.DragDrop.AddDragEnterHandler(dropArea, (_, e) =>
        {
            if (HasAssetData(e))
            {
                dropArea.BorderBrush = highlightBrush;
                dropArea.BorderThickness = new Thickness(2);
                e.DragEffects = DragDropEffects.Copy;
            }
            else
            {
                e.DragEffects = DragDropEffects.None;
            }
        });

        // 拖拽悬停 → 保持高亮
        Avalonia.Input.DragDrop.AddDragOverHandler(dropArea, (_, e) =>
        {
            e.DragEffects = HasAssetData(e) ? DragDropEffects.Copy : DragDropEffects.None;
        });

        // 拖拽离开 → 取消高亮
        Avalonia.Input.DragDrop.AddDragLeaveHandler(dropArea, (_, _) =>
        {
            dropArea.BorderBrush = EditorTheme.Border;
            dropArea.BorderThickness = new Thickness(1);
        });

        // 拖拽放下 → 接收资产
        Avalonia.Input.DragDrop.AddDropHandler(dropArea, (_, e) =>
        {
            // 恢复边框
            dropArea.BorderBrush = EditorTheme.Border;
            dropArea.BorderThickness = new Thickness(1);

            var assetPath = AssetDragFormats.GetAssetPathFromDrag(e);
            if (!string.IsNullOrEmpty(assetPath))
            {
                // 显示资产名称
                var name = System.IO.Path.GetFileNameWithoutExtension(assetPath);
                assetLabel.Text = name;
                assetLabel.Foreground = EditorTheme.TextPrimary;

                onAssetDropped?.Invoke(assetPath);
                e.DragEffects = DragDropEffects.Copy;
            }
            else
            {
                e.DragEffects = DragDropEffects.None;
            }
        });

        // 允许接收拖拽
        Avalonia.Input.DragDrop.SetAllowDrop(dropArea, true);

        return dropArea;
    }
}
