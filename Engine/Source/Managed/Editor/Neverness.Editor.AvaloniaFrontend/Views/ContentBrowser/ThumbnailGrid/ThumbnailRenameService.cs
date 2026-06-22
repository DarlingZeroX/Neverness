using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ThumbnailGrid;

/// <summary>
/// 缩略图内联重命名服务——处理文件名的内联编辑。
/// </summary>
internal sealed class ThumbnailRenameService
{
    private TextBox? _activeTextBox;
    private TextBlock? _activeNameLabel;
    private string? _activePath;
    private string? _activeCurrentName;

    /// <summary>重命名提交事件（path, newName）。</summary>
    internal event Action<string, string>? OnRenameCommitted;

    /// <summary>是否有正在进行的重命名。</summary>
    internal bool IsRenaming => _activeTextBox != null;

    /// <summary>开始内联重命名——将文件名 TextBlock 替换为 TextBox。</summary>
    internal void BeginRename(Control thumbnail, string path, string currentName)
    {
        // 如果已有重命名在进行，先取消
        if (_activeTextBox != null)
            CancelRename();

        // 找到 nameLabel（通过 Tag 标记）
        var nameLabel = FindNameLabel(thumbnail);
        if (nameLabel == null) return;

        // 找到 nameLabel 的父容器（StackPanel）
        if (nameLabel.Parent is not StackPanel textArea) return;

        // 隐藏原文件名
        nameLabel.IsVisible = false;

        // 创建内联编辑框
        var textBox = new TextBox
        {
            Text = currentName,
            FontSize = 11,
            MinWidth = 60,
            Height = ThumbNameHeight,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(-2, 0, -2, 0),
            Padding = new Thickness(2, 0),
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            Foreground = TextBright,
            BorderBrush = new SolidColorBrush(Color.Parse("#FF007ACC")),
            BorderThickness = new Thickness(1),
        };

        // 选中文件名（不含扩展名）
        var dotIndex = currentName.LastIndexOf('.');
        textBox.SelectionStart = 0;
        textBox.SelectionEnd = dotIndex > 0 ? dotIndex : currentName.Length;

        // 保存状态
        _activeTextBox = textBox;
        _activeNameLabel = nameLabel;
        _activePath = path;
        _activeCurrentName = currentName;

        // 提交重命名
        void CommitRename()
        {
            var newName = textBox.Text?.Trim() ?? "";
            if (!string.IsNullOrEmpty(newName) && newName != currentName)
            {
                OnRenameCommitted?.Invoke(path, newName);
            }
            CancelRename();
        }

        textBox.KeyDown += (_, e) =>
        {
            if (e.Key == Key.Enter)
            {
                CommitRename();
                e.Handled = true;
            }
            else if (e.Key == Key.Escape)
            {
                CancelRename();
                e.Handled = true;
            }
        };

        // 失焦也提交
        textBox.LostFocus += (_, _) => CommitRename();

        textArea.Children.Add(textBox);

        // 聚焦并延迟选中（等控件加载完）
        textBox.AttachedToVisualTree += (_, _) =>
        {
            textBox.Focus();
            textBox.SelectAll();
        };
    }

    /// <summary>取消重命名——移除 TextBox，恢复 TextBlock。</summary>
    internal void CancelRename()
    {
        if (_activeTextBox == null) return;

        if (_activeTextBox.Parent is StackPanel textArea)
            textArea.Children.Remove(_activeTextBox);

        if (_activeNameLabel != null)
            _activeNameLabel.IsVisible = true;

        _activeTextBox = null;
        _activeNameLabel = null;
        _activePath = null;
        _activeCurrentName = null;
    }

    /// <summary>查找缩略图中的 NameLabel。</summary>
    private static TextBlock? FindNameLabel(Control thumbnail)
    {
        // 递归查找 Tag 为 "NameLabel" 的 TextBlock
        return FindNameLabelRecursive(thumbnail);
    }

    private static TextBlock? FindNameLabelRecursive(Control control)
    {
        if (control is TextBlock tb && tb.Tag is string tag && tag == "NameLabel")
            return tb;

        if (control is Panel panel)
        {
            foreach (var child in panel.Children)
            {
                if (child is Control childCtrl)
                {
                    var result = FindNameLabelRecursive(childCtrl);
                    if (result != null) return result;
                }
            }
        }
        else if (control is Decorator decorator && decorator.Child is Control decoratorChild)
        {
            return FindNameLabelRecursive(decoratorChild);
        }
        else if (control is ContentControl contentCtrl && contentCtrl.Content is Control contentChild)
        {
            return FindNameLabelRecursive(contentChild);
        }

        return null;
    }
}
