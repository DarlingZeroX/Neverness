namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 编辑器本地化文本辅助类——提供简洁的本地化文本访问接口。
///
/// 使用方式：
/// <code>
/// string text = EditorText.Get("Open");
/// string text = EditorText.Get("The texture is loaded successfully: %s [Width: %d  Height: %d]", fileName, width, height);
/// </code>
/// </summary>
public static class EditorText
{
    /// <summary>
    /// 获取本地化文本。
    /// </summary>
    /// <param name="key">文本 Key（如 "Open"、"Cancel"）。</param>
    /// <returns>本地化文本，如果未找到则返回 key 本身。</returns>
    public static string Get(string key)
    {
        return EditorResourceCache.Instance.GetText(key);
    }

    /// <summary>
    /// 获取本地化文本（支持格式化参数）。
    /// </summary>
    /// <param name="key">文本 Key。</param>
    /// <param name="args">格式化参数。</param>
    /// <returns>格式化后的本地化文本。</returns>
    public static string Get(string key, params object[] args)
    {
        return EditorResourceCache.Instance.GetText(key, args);
    }

    /// <summary>
    /// 切换语言。
    /// </summary>
    /// <param name="languageCode">语言代码（如 "ZH_CN"、"EN_US"）。</param>
    public static void SetLanguage(string languageCode)
    {
        EditorResourceCache.Instance.SetLanguage(languageCode);
    }

    /// <summary>
    /// 获取当前语言代码。
    /// </summary>
    public static string CurrentLanguage => EditorResourceCache.Instance.CurrentLanguage;

    /// <summary>
    /// 预加载本地化文本（启动时调用）。
    /// </summary>
    /// <param name="languageCode">语言代码，null 使用当前语言。</param>
    public static void Preload(string? languageCode = null)
    {
        EditorResourceCache.Instance.PreloadLocalization(languageCode);
    }

    // ── 常用文本快捷访问 ──

    /// <summary>"打开"。</summary>
    public static string Open => Get("Open");

    /// <summary>"新建"。</summary>
    public static string New => Get("New");

    /// <summary>"保存"。</summary>
    public static string Save => Get("Save");

    /// <summary>"取消"。</summary>
    public static string Cancel => Get("Cancel");

    /// <summary>"关闭"。</summary>
    public static string Close => Get("Close");

    /// <summary>"确认"。</summary>
    public static string Ok => Get("Ok");

    /// <summary>"删除"。</summary>
    public static string Delete => Get("Remove");

    /// <summary>"重命名"。</summary>
    public static string Rename => Get("Rename");

    /// <summary>"刷新"。</summary>
    public static string Refresh => Get("Refresh");

    /// <summary>"设置"。</summary>
    public static string Settings => Get("Setting");

    /// <summary>"项目"。</summary>
    public static string Project => Get("Project");

    /// <summary>"场景"。</summary>
    public static string Scene => Get("Scene");

    /// <summary>"纹理"。</summary>
    public static string Texture => Get("Texture");

    /// <summary>"消息"。</summary>
    public static string Info => Get("Info");

    /// <summary>"警告"。</summary>
    public static string Warning => Get("Warning");

    /// <summary>"成功"。</summary>
    public static string Success => Get("Success");

    /// <summary>"错误"。</summary>
    public static string Error => Get("Error");

    /// <summary>"复制"。</summary>
    public static string Copy => Get("Copy");

    /// <summary>"粘贴"。</summary>
    public static string Paste => Get("Paste");
}
