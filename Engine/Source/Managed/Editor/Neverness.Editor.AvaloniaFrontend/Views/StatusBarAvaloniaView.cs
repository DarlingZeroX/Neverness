using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 状态栏 Avalonia View——显示当前状态、进度、警告。
///
/// 与 ImGui 的状态栏对应。
/// 显示编辑器状态信息。
/// </summary>
public class StatusBarAvaloniaView : AvaloniaViewBase
{
    private DockPanel? _statusBar;
    private TextBlock? _statusText;
    private TextBlock? _fpsText;
    private TextBlock? _warningText;

    public StatusBarAvaloniaView() : base("StatusBar")
    {
    }

    public override Type ViewModelType => typeof(object); // 状态栏没有 ViewModel

    public override void Bind(object viewModel)
    {
        // 状态栏不绑定 ViewModel
        CreateStatusBar();
    }

    public override void Unbind()
    {
        _statusBar = null;
        _statusText = null;
        _fpsText = null;
        _warningText = null;
    }

    /// <summary>获取状态栏控件。</summary>
    public DockPanel? GetStatusBar() => _statusBar;

    /// <summary>创建状态栏。</summary>
    private void CreateStatusBar()
    {
        _statusBar = new DockPanel
        {
            Background = new SolidColorBrush(Color.Parse("#FF007ACC")),
            Height = 24,
        };

        // 左侧：状态文本
        _statusText = new TextBlock
        {
            Text = "Ready",
            FontSize = 11,
            Foreground = Brushes.White,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
        };
        Avalonia.Controls.DockPanel.SetDock(_statusText, Avalonia.Controls.Dock.Left);
        _statusBar.Children.Add(_statusText);

        // 右侧：FPS
        _fpsText = new TextBlock
        {
            Text = "60 FPS",
            FontSize = 11,
            Foreground = Brushes.White,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
        };
        Avalonia.Controls.DockPanel.SetDock(_fpsText, Avalonia.Controls.Dock.Right);
        _statusBar.Children.Add(_fpsText);

        // 右侧：警告
        _warningText = new TextBlock
        {
            Text = "",
            FontSize = 11,
            Foreground = new SolidColorBrush(Color.Parse("#FFFFC107")),
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
        };
        Avalonia.Controls.DockPanel.SetDock(_warningText, Avalonia.Controls.Dock.Right);
        _statusBar.Children.Add(_warningText);
    }

    /// <summary>更新状态文本。</summary>
    public void SetStatus(string text)
    {
        if (_statusText != null)
            _statusText.Text = text;
    }

    /// <summary>更新 FPS 显示。</summary>
    public void SetFps(int fps)
    {
        if (_fpsText != null)
            _fpsText.Text = $"{fps} FPS";
    }

    /// <summary>显示警告。</summary>
    public void SetWarning(string? warning)
    {
        if (_warningText != null)
        {
            _warningText.Text = warning ?? "";
            _warningText.IsVisible = !string.IsNullOrEmpty(warning);
        }
    }
}
