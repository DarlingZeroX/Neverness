using Avalonia;

namespace Neverness.Editor.AvaloniaFrontend;

/// <summary>
/// Avalonia 应用启动入口。
///
/// 由 AvaloniaFrontendModule 通过独立线程调用，
/// 启动 Avalonia 桌面应用。
/// </summary>
internal static class Program
{
    /// <summary>Avalonia 应用实例（供外部控制生命周期）。</summary>
    private static AppBuilder? _appBuilder;

    /// <summary>启动 Avalonia 应用（阻塞，需在独立线程调用）。</summary>
    public static void RunAvaloniaApp()
    {
        _appBuilder = BuildAvaloniaApp();
        _appBuilder.StartWithClassicDesktopLifetime(args: Array.Empty<string>());
    }

    /// <summary>构建 Avalonia 应用配置。</summary>
    public static AppBuilder BuildAvaloniaApp()
    {
        return AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .WithInterFont()
            .LogToTrace();
    }

    /// <summary>关闭 Avalonia 应用。</summary>
    public static void Shutdown()
    {
        // Avalonia 的 ClassicDesktopLifetime 会在窗口关闭时自动退出
        // 此方法用于外部强制退出
        if (_appBuilder != null)
        {
            // 触发关闭
            if (Avalonia.Application.Current?.ApplicationLifetime
                is Avalonia.Controls.ApplicationLifetimes.IClassicDesktopStyleApplicationLifetime lifetime)
            {
                lifetime.Shutdown();
            }
        }
    }
}
