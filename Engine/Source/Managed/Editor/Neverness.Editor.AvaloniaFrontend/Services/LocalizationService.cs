using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// 本地化服务——包装 EditorText，提供 Avalonia 绑定接口。
///
/// 使用现有的 EditorText 机制，支持运行时语言切换。
/// 当语言切换时，通知所有绑定的 UI 更新。
/// </summary>
public class LocalizationService
{
    private string _currentLanguage = "ZH_CN";

    /// <summary>当前语言。</summary>
    public string CurrentLanguage => _currentLanguage;

    /// <summary>语言变更事件。</summary>
    public event Action<string>? LanguageChanged;

    /// <summary>
    /// 获取本地化文本。
    /// </summary>
    public string Get(string key)
    {
        return EditorText.Get(key);
    }

    /// <summary>
    /// 获取本地化文本（支持格式化参数）。
    /// </summary>
    public string Get(string key, params object[] args)
    {
        return EditorText.Get(key, args);
    }

    /// <summary>
    /// 切换语言。
    /// </summary>
    public void SetLanguage(string languageCode)
    {
        if (_currentLanguage == languageCode)
            return;

        _currentLanguage = languageCode;
        EditorText.SetLanguage(languageCode);

        // 触发语言变更事件
        LanguageChanged?.Invoke(languageCode);

        Console.WriteLine($"[LocalizationService] 语言已切换: {languageCode}");
    }

    /// <summary>
    /// 切换到下一个语言。
    /// </summary>
    public void ToggleLanguage()
    {
        var nextLanguage = _currentLanguage switch
        {
            "ZH_CN" => "EN_US",
            "EN_US" => "ZH_CN",
            _ => "ZH_CN"
        };
        SetLanguage(nextLanguage);
    }

    /// <summary>
    /// 预加载本地化文本。
    /// </summary>
    public void Preload(string? languageCode = null)
    {
        EditorText.Preload(languageCode);
    }
}
