using Avalonia;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using NevernessLauncher.Contracts;
using NevernessLauncher.Infrastructure;
using NevernessLauncher.Infrastructure.Process;
using NevernessLauncher.Services;
using NevernessLauncher.ViewModels;
using NevernessLauncher.ViewModels.Home;
using NevernessLauncher.ViewModels.Settings;
using System;

namespace NevernessLauncher
{
    internal sealed class Program
    {
        // 全局 ServiceProvider
        public static IServiceProvider? ServiceProvider { get; private set; }

        [STAThread]
        public static void Main(string[] args)
        {
            // 构建 Host
            var host = CreateHostBuilder(args).Build();

            // 启动 Host
            host.Start();

            // 保存 ServiceProvider
            ServiceProvider = host.Services;

            // 运行 Avalonia 应用
            BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);

            // 停止 Host
            host.StopAsync().GetAwaiter().GetResult();
        }

        public static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureServices((context, services) =>
                {
                    // 注册 Infrastructure 服务
                    services.AddSingleton<IFileSystem, FileSystem>();
                    services.AddSingleton<IProcessLauncher, ProcessLauncher>();
                    services.AddSingleton<ILogService, LogService>();

                    // 注册 Configuration 服务
                    services.AddSingleton<IConfigurationService, ConfigurationService>();

                    // 注册 Core 服务
                    services.AddSingleton<IEngineService, EngineService>();
                    services.AddSingleton<IRecentProjectService, RecentProjectService>();
                    services.AddSingleton<IProjectService, ProjectService>();
                    services.AddSingleton<ILaunchService, LaunchService>();

                    // 注册 ViewModels
                    services.AddSingleton<MainWindowViewModel>();
                    services.AddTransient<HomePageViewModel>();
                    services.AddTransient<SettingsPageViewModel>();
                });

        public static AppBuilder BuildAvaloniaApp()
        {
            return AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .WithInterFont()
                .LogToTrace();
        }
    }
}
