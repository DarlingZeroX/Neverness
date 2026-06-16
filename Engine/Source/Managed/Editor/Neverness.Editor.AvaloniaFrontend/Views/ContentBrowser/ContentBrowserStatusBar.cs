using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Editor.Core.ViewModels;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器状态栏——显示当前路径和选中数量。
/// </summary>
internal sealed class ContentBrowserStatusBar
{
    private readonly ContentBrowserViewModel _viewModel;

    private TextBlock? _statusText;
    private TextBlock? _selectionStatus;

    internal ContentBrowserStatusBar(ContentBrowserViewModel viewModel)
    {
        _viewModel = viewModel;
    }

    /// <summary>创建状态栏控件。</summary>
    internal Control Create()
    {
        var bar = new DockPanel
        {
            Background = BgStatusBar,
            Height = 24,
            Margin = new Thickness(0),
        };

        _statusText = new TextBlock
        {
            Text = "",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(12, 0, 0, 0),
        };
        DockPanel.SetDock(_statusText, Avalonia.Controls.Dock.Left);
        bar.Children.Add(_statusText);

        _selectionStatus = new TextBlock
        {
            Text = "",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(0, 0, 12, 0),
            HorizontalAlignment = HorizontalAlignment.Right,
        };
        DockPanel.SetDock(_selectionStatus, Avalonia.Controls.Dock.Right);
        bar.Children.Add(_selectionStatus);

        return bar;
    }

    /// <summary>更新状态栏显示。</summary>
    internal void Update(int selectedCount)
    {
        if (_statusText != null)
        {
            var dir = _viewModel.CurrentDirectory ?? "";
            var assetDir = _viewModel.AssetDirectory ?? "";
            _statusText.Text = string.IsNullOrEmpty(assetDir) ? dir : System.IO.Path.GetRelativePath(assetDir, dir);
        }

        if (_selectionStatus != null)
        {
            _selectionStatus.Text = selectedCount > 0 ? $"{selectedCount} item{(selectedCount > 1 ? "s" : "")} selected" : "";
        }
    }
}
