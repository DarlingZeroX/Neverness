using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Reactive;
using Neverness.Editor.Core.ViewModels;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 控制台 Avalonia View——显示日志消息。
///
/// 实现细节：
/// - ListBox 显示日志条目（虚拟化）
/// - 工具栏：清空、过滤级别、自动滚动
/// - 按日志级别着色
/// - 绑定到 ConsolePanelViewModel
/// </summary>
public class ConsolePanelAvaloniaView : AvaloniaViewBase
{
    private ConsolePanelViewModel? _viewModel;
    private ListBox? _logListBox;
    private TextBox? _filterTextBox;

    public ConsolePanelAvaloniaView() : base("Console")
    {
    }

    public override Type ViewModelType => typeof(ConsolePanelViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ConsolePanelViewModel)viewModel;

        // 创建 Avalonia 控件树
        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        Avalonia.Controls.DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 过滤栏 ──
        var filterBar = CreateFilterBar();
        Avalonia.Controls.DockPanel.SetDock(filterBar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(filterBar);

        // ── 日志列表 ──
        _logListBox = new ListBox
        {
            ItemsSource = _viewModel.FilteredEntries,
            ItemTemplate = new LogEntryTemplate(),
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            BorderThickness = new Avalonia.Thickness(0),
        };

        panel.Children.Add(_logListBox);
    }

    public override void Unbind()
    {
        _viewModel = null;
        _logListBox = null;
        _filterTextBox = null;
    }

    /// <summary>创建工具栏。</summary>
    private StackPanel CreateToolbar()
    {
        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4),
        };

        // 清空按钮
        var clearButton = new Button
        {
            Content = "Clear",
            Width = 60,
            Height = 24,
        };
        clearButton.Click += (_, _) => _viewModel?.Clear();
        toolbar.Children.Add(clearButton);

        // 自动滚动复选框
        var autoScrollCheckBox = new CheckBox
        {
            Content = "Auto Scroll",
            IsChecked = _viewModel?.AutoScroll ?? true,
            VerticalAlignment = VerticalAlignment.Center,
        };
        autoScrollCheckBox.IsCheckedChanged += (_, _) =>
        {
            if (_viewModel != null && autoScrollCheckBox.IsChecked.HasValue)
                _viewModel.AutoScroll = autoScrollCheckBox.IsChecked.Value;
        };
        toolbar.Children.Add(autoScrollCheckBox);

        return toolbar;
    }

    /// <summary>创建过滤栏。</summary>
    private StackPanel CreateFilterBar()
    {
        var filterBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4, 0, 4, 4),
        };

        // 过滤文本框
        _filterTextBox = new TextBox
        {
            Watermark = "Filter logs...",
            Width = 200,
            Height = 24,
        };
        _filterTextBox.TextChanged += (_, e) =>
        {
            if (_viewModel != null)
                _viewModel.FilterText = _filterTextBox.Text ?? "";
        };
        filterBar.Children.Add(_filterTextBox);

        // 日志级别过滤
        var levelCombo = new ComboBox
        {
            Width = 100,
            Height = 24,
            ItemsSource = new[] { "All", "Debug", "Info", "Warning", "Error", "Fatal" },
            SelectedIndex = 0,
        };
        levelCombo.SelectionChanged += (_, _) =>
        {
            if (_viewModel != null)
            {
                _viewModel.FilterLevel = levelCombo.SelectedIndex switch
                {
                    1 => LogLevel.Debug,
                    2 => LogLevel.Info,
                    3 => LogLevel.Warning,
                    4 => LogLevel.Error,
                    5 => LogLevel.Fatal,
                    _ => LogLevel.All
                };
            }
        };
        filterBar.Children.Add(levelCombo);

        return filterBar;
    }
}

/// <summary>
/// 日志条目模板——按级别着色。
/// </summary>
internal class LogEntryTemplate : IDataTemplate
{
    public Control? Build(object? param)
    {
        if (param is not LogEntry entry)
            return null;

        var textBlock = new TextBlock
        {
            Text = $"[{entry.Timestamp:HH:mm:ss}] [{entry.Level}] {entry.Message}",
            FontFamily = new FontFamily("Consolas"),
            FontSize = 12,
            Margin = new Avalonia.Thickness(4, 1),
        };

        // 按日志级别着色
        textBlock.Foreground = entry.Level switch
        {
            LogLevel.Error or LogLevel.Fatal => new SolidColorBrush(Color.Parse("#FFF44336")),
            LogLevel.Warning => new SolidColorBrush(Color.Parse("#FFFFC107")),
            LogLevel.Debug => new SolidColorBrush(Color.Parse("#FF999999")),
            _ => new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };

        return textBlock;
    }

    public bool Match(object? data)
    {
        return data is LogEntry;
    }
}
