using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Settings.Private;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views;

/// <summary>
/// 设置窗口——左侧 TreeView 分类导航，右侧显示设置面板。
/// 根据 SettingsScope 过滤显示 Project Settings 或 Preferences。
/// </summary>
internal partial class SettingsWindow : Window
{
    private readonly SettingsServiceImpl _service;
    private readonly SettingsScope _scope;
    private SettingsTable? _currentTable;

    public SettingsWindow(SettingsServiceImpl service, SettingsScope scope)
    {
        _service = service;
        _scope = scope;
        InitializeComponent();
        BuildTreeView();
        WireEvents();

        // 默认选中第一个
        if (SettingsTree.Items.Count > 0 && SettingsTree.Items[0] is TreeViewItem firstItem)
        {
            firstItem.IsSelected = true;
        }
    }

    /// <summary>构建左侧 TreeView。</summary>
    private void BuildTreeView()
    {
        // 按范围过滤注册条目
        var registrations = _service.GetRegistrationsByScope(_scope);

        // 提取当前范围内的分类
        var categories = registrations
            .Select(r => r.Descriptor.Category)
            .Where(c => !string.IsNullOrEmpty(c))
            .Distinct(StringComparer.Ordinal)
            .Cast<string>()
            .ToList();

        // 按分类分组
        var categoryGroups = new Dictionary<string, List<SettingsRegistry.Registration>>();

        foreach (var reg in registrations)
        {
            var category = reg.Descriptor.Category ?? "其他";
            if (!categoryGroups.TryGetValue(category, out var list))
            {
                list = new List<SettingsRegistry.Registration>();
                categoryGroups[category] = list;
            }
            list.Add(reg);
        }

        // 按分类顺序构建 TreeView
        foreach (var category in categories)
        {
            if (!categoryGroups.TryGetValue(category, out var items))
                continue;

            var categoryItem = new TreeViewItem
            {
                Header = CreateCategoryHeader(category),
                IsExpanded = true,
            };

            foreach (var reg in items)
            {
                var tableItem = new TreeViewItem
                {
                    Header = CreateTableHeader(reg.Table.DisplayName),
                    Tag = reg.Table,
                };
                tableItem.Tapped += OnTableSelected;
                categoryItem.Items.Add(tableItem);
            }

            SettingsTree.Items.Add(categoryItem);
        }

        // 没有分类的设置表直接放在根级
        foreach (var reg in registrations)
        {
            if (reg.Descriptor.Category != null)
                continue;

            var tableItem = new TreeViewItem
            {
                Header = CreateTableHeader(reg.Table.DisplayName),
                Tag = reg.Table,
            };
            tableItem.Tapped += OnTableSelected;
            SettingsTree.Items.Add(tableItem);
        }
    }

    /// <summary>创建分类头（粗体）。</summary>
    private static TextBlock CreateCategoryHeader(string text)
    {
        return new TextBlock
        {
            Text = text,
            FontWeight = FontWeight.Bold,
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
            Margin = new Thickness(8, 6, 4, 2),
        };
    }

    /// <summary>创建设置表头（带悬停效果）。</summary>
    private static Control CreateTableHeader(string text)
    {
        var border = new Border
        {
            CornerRadius = new CornerRadius(4),
            Padding = new Thickness(8, 4),
            Margin = new Thickness(4, 1),
            Child = new TextBlock
            {
                Text = text,
                FontSize = 12,
                Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            }
        };

        border.PointerEntered += (_, _) =>
            border.Background = new SolidColorBrush(Color.Parse("#33FFFFFF"));
        border.PointerExited += (_, _) =>
            border.Background = Brushes.Transparent;

        return border;
    }

    /// <summary>绑定事件。</summary>
    private void WireEvents()
    {
        ResetButton.Click += OnResetClick;
        ApplyButton.Click += OnApplyClick;
        OkButton.Click += OnOkClick;
        CancelButton.Click += OnCancelClick;
        CloseButton.Click += (_, _) => Close();

        // 标题栏拖动
        TitleBar.PointerPressed += (_, e) =>
        {
            if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
                BeginMoveDrag(e);
        };
    }

    /// <summary>TreeView 选中项变更。</summary>
    private void OnTableSelected(object? sender, RoutedEventArgs e)
    {
        if (sender is TreeViewItem item && item.Tag is SettingsTable table)
        {
            _currentTable = table;
            ShowTableContent(table);
        }
    }

    /// <summary>显示设置表内容。</summary>
    private void ShowTableContent(SettingsTable table)
    {
        // 检查是否有自定义页面提供者
        var customProvider = _service.GetPageProvider(table.TableId);
        if (customProvider != null)
        {
            SettingsContent.Content = customProvider.CreatePage(table);
            return;
        }

        // 使用描述符自动生成面板
        var descriptor = _service.GetDescriptor(table.TableId);
        if (descriptor != null)
        {
            SettingsContent.Content = SettingsPanelBuilder.Create(descriptor, table, _service);
        }
        else
        {
            // 没有描述符，显示占位
            SettingsContent.Content = new TextBlock
            {
                Text = $"设置表 '{table.DisplayName}' 没有描述符，无法自动生成 UI。",
                Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
            };
        }
    }

    // ── 按钮事件 ──

    private void OnResetClick(object? sender, RoutedEventArgs e)
    {
        _currentTable?.ResetDefaults();
        if (_currentTable != null)
            ShowTableContent(_currentTable);
    }

    private void OnApplyClick(object? sender, RoutedEventArgs e)
    {
        _service.SaveAll();
    }

    private void OnOkClick(object? sender, RoutedEventArgs e)
    {
        _service.SaveAll();
        Close();
    }

    private void OnCancelClick(object? sender, RoutedEventArgs e)
    {
        Close();
    }
}
