using Avalonia.Controls;
using Avalonia.Interactivity;
using NevernessLauncher.ViewModels.Settings;

namespace NevernessLauncher.Views.Settings
{
    public partial class SettingsPage : UserControl
    {
        public SettingsPage()
        {
            InitializeComponent();

            // 获取 Theme RadioButton 并绑定事件
            var themeDark = this.FindControl<RadioButton>("ThemeDark");
            var themeLight = this.FindControl<RadioButton>("ThemeLight");
            var themeSystem = this.FindControl<RadioButton>("ThemeSystem");

            if (themeDark != null) themeDark.IsCheckedChanged += OnThemeChanged;
            if (themeLight != null) themeLight.IsCheckedChanged += OnThemeChanged;
            if (themeSystem != null) themeSystem.IsCheckedChanged += OnThemeChanged;
        }

        private void OnThemeChanged(object? sender, RoutedEventArgs e)
        {
            if (sender is RadioButton radioButton && radioButton.IsChecked == true)
            {
                var viewModel = DataContext as SettingsPageViewModel;
                if (viewModel == null) return;

                var theme = radioButton.Name switch
                {
                    "ThemeDark" => "Dark",
                    "ThemeLight" => "Light",
                    "ThemeSystem" => "System",
                    _ => "Dark"
                };

                viewModel.Theme = theme;
            }
        }

        protected override void OnDataContextChanged(System.EventArgs e)
        {
            base.OnDataContextChanged(e);

            // 根据当前 Theme 设置 RadioButton 选中状态
            if (DataContext is SettingsPageViewModel viewModel)
            {
                UpdateThemeRadioButtons(viewModel.Theme);
            }
        }

        private void UpdateThemeRadioButtons(string theme)
        {
            var themeDark = this.FindControl<RadioButton>("ThemeDark");
            var themeLight = this.FindControl<RadioButton>("ThemeLight");
            var themeSystem = this.FindControl<RadioButton>("ThemeSystem");

            if (themeDark != null) themeDark.IsChecked = theme == "Dark";
            if (themeLight != null) themeLight.IsChecked = theme == "Light";
            if (themeSystem != null) themeSystem.IsChecked = theme == "System";
        }
    }
}
