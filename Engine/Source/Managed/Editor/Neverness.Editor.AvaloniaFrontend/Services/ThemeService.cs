using Avalonia;
using Avalonia.Styling;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// 主题服务——管理编辑器主题切换。
///
/// 支持 Dark/Light 两个主题。
/// 运行时切换主题，通过 Avalonia 的资源系统实现。
/// </summary>
public class ThemeService
{
    /// <summary>可用主题。</summary>
    public enum Theme
    {
        Dark,
        Light
    }

    private Theme _currentTheme = Theme.Dark;

    /// <summary>当前主题。</summary>
    public Theme CurrentTheme => _currentTheme;

    /// <summary>主题变更事件。</summary>
    public event Action<Theme>? ThemeChanged;

    /// <summary>
    /// 切换主题。
    /// </summary>
    public void SetTheme(Theme theme)
    {
        if (_currentTheme == theme)
            return;

        _currentTheme = theme;

        // 切换 Avalonia 主题变体
        if (Application.Current != null)
        {
            Application.Current.RequestedThemeVariant = theme switch
            {
                Theme.Dark => ThemeVariant.Dark,
                Theme.Light => ThemeVariant.Light,
                _ => ThemeVariant.Dark
            };
        }

        // 加载对应主题的资源字典
        LoadThemeResources(theme);

        // 触发事件
        ThemeChanged?.Invoke(theme);

        Console.WriteLine($"[ThemeService] 主题已切换: {theme}");
    }

    /// <summary>
    /// 切换到下一个主题。
    /// </summary>
    public void ToggleTheme()
    {
        var nextTheme = _currentTheme switch
        {
            Theme.Dark => Theme.Light,
            Theme.Light => Theme.Dark,
            _ => Theme.Dark
        };
        SetTheme(nextTheme);
    }

    /// <summary>
    /// 加载主题资源字典。
    /// </summary>
    private void LoadThemeResources(Theme theme)
    {
        if (Application.Current == null) return;

        var resourceDict = Application.Current.Resources;

        // 移除旧的主题资源
        // TODO: 实现资源字典切换

        // 加载新的主题资源
        var themeUri = theme switch
        {
            Theme.Dark => new Uri("avares://Neverness.Editor.AvaloniaFrontend/Themes/DarkTheme.axaml"),
            Theme.Light => new Uri("avares://Neverness.Editor.AvaloniaFrontend/Themes/LightTheme.axaml"),
            _ => new Uri("avares://Neverness.Editor.AvaloniaFrontend/Themes/DarkTheme.axaml")
        };

        try
        {
            // 注意：Avalonia 的资源系统在运行时切换需要特殊处理
            // 此处为简化实现，实际可能需要更复杂的资源管理
            Console.WriteLine($"[ThemeService] 加载主题资源: {themeUri}");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ThemeService] 加载主题资源失败: {ex.Message}");
        }
    }

    /// <summary>
    /// 持久化主题设置。
    /// </summary>
    public void SaveThemePreference()
    {
        // TODO: 保存到用户设置文件
        Console.WriteLine($"[ThemeService] 保存主题偏好: {_currentTheme}");
    }

    /// <summary>
    /// 加载主题设置。
    /// </summary>
    public void LoadThemePreference()
    {
        // TODO: 从用户设置文件加载
        Console.WriteLine("[ThemeService] 加载主题偏好");
    }
}
