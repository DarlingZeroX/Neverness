using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Styling;
using Microsoft.Extensions.DependencyInjection;
using NevernessLauncher.Contracts;
using NevernessLauncher.ViewModels;
using NevernessLauncher.Views;

namespace NevernessLauncher
{
    public partial class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
            {
                var serviceProvider = Program.ServiceProvider;
                if (serviceProvider != null)
                {
                    // 加载并应用保存的 Theme 设置
                    var configService = serviceProvider.GetService<IConfigurationService>();
                    if (configService != null)
                    {
                        var settings = configService.LoadUserSettings();
                        ApplyTheme(settings.Theme);
                    }

                    var mainWindowViewModel = serviceProvider.GetRequiredService<MainWindowViewModel>();
                    desktop.MainWindow = new MainWindow
                    {
                        DataContext = mainWindowViewModel
                    };
                }
            }

            base.OnFrameworkInitializationCompleted();
        }

        private void ApplyTheme(string theme)
        {
            var themeVariant = theme switch
            {
                "Dark" => ThemeVariant.Dark,
                "Light" => ThemeVariant.Light,
                "System" => ThemeVariant.Default,
                _ => ThemeVariant.Dark
            };

            RequestedThemeVariant = themeVariant;
        }
    }
}
