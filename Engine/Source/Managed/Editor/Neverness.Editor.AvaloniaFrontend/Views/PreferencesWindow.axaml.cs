using Avalonia.Controls;
using Avalonia.Interactivity;
using Neverness.Editor.Core.Public;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 偏好设置窗口——配置首选 IDE。
/// </summary>
public partial class PreferencesWindow : Window
{
    private readonly IPreferencesService _preferencesService;

    public PreferencesWindow(IPreferencesService preferencesService)
    {
        _preferencesService = preferencesService;
        InitializeComponent();
        LoadSettings();
        WireEvents();
    }

    /// <summary>加载当前设置到 UI。</summary>
    private void LoadSettings()
    {
        // 填充 ComboBox 选项
        IDEComboBox.Items.Add("Visual Studio");
        IDEComboBox.Items.Add("VS Code");

        // 设置当前选中项
        IDEComboBox.SelectedIndex = _preferencesService.PreferredIDE switch
        {
            IDEPreference.VisualStudio => 0,
            IDEPreference.VSCode => 1,
            _ => 0
        };
    }

    /// <summary>绑定事件。</summary>
    private void WireEvents()
    {
        SaveButton.Click += OnSaveClick;
        CancelButton.Click += OnCancelClick;
    }

    /// <summary>保存按钮点击。</summary>
    private void OnSaveClick(object? sender, RoutedEventArgs e)
    {
        _preferencesService.PreferredIDE = IDEComboBox.SelectedIndex switch
        {
            0 => IDEPreference.VisualStudio,
            1 => IDEPreference.VSCode,
            _ => IDEPreference.VisualStudio
        };

        _preferencesService.Save();
        Close();
    }

    /// <summary>取消按钮点击。</summary>
    private void OnCancelClick(object? sender, RoutedEventArgs e)
    {
        Close();
    }
}
