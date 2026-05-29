namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 编辑器图标常量——提供常用图标的文件名和便捷访问方法。
///
/// 使用方式：
/// <code>
/// ulong texHandle = EditorIcons.GetTexture(EditorIcons.Folder);
/// ulong texHandle = EditorIcons.GetTexture(EditorIcons.File);
/// </code>
/// </summary>
public static class EditorIcons
{
    // ── 图标文件名常量 ──

    /// <summary>文件夹图标。</summary>
    public const string Folder = "folder.png";

    /// <summary>通用文件图标。</summary>
    public const string File = "file.png";

    /// <summary>引擎图标。</summary>
    public const string Engine = "engineIcon.png";

    /// <summary>场景图标。</summary>
    public const string Scene = "scene.png";

    /// <summary>图像/纹理图标。</summary>
    public const string Image = "image.png";

    /// <summary>音频/声音图标。</summary>
    public const string Sound = "sound.png";

    /// <summary>视频图标。</summary>
    public const string Video = "video.png";

    /// <summary>Lua 脚本图标。</summary>
    public const string Lua = "lua.png";

    /// <summary>CSS 图标。</summary>
    public const string Css = "css.png";

    /// <summary>HTML 图标。</summary>
    public const string Html = "html.png";

    /// <summary>GalGame 剧情脚本图标。</summary>
    public const string GalStoryScript = "galStoryScript.png";

    /// <summary>向左箭头图标。</summary>
    public const string Left = "left.png";

    /// <summary>向右箭头图标。</summary>
    public const string Right = "right.png";

    /// <summary>刷新图标。</summary>
    public const string Reload = "reload.png";

    // ── 便捷方法 ──

    /// <summary>
    /// 获取图标的 ImGui 纹理句柄。
    /// </summary>
    /// <param name="iconName">图标文件名（使用 EditorIcons 常量）。</param>
    /// <returns>ImGui 纹理句柄，0 = 加载失败。</returns>
    public static ulong GetTexture(string iconName)
    {
        return EditorResourceCache.Instance.GetIconTexture(iconName);
    }

    /// <summary>
    /// 获取图标尺寸。
    /// </summary>
    /// <param name="iconName">图标文件名。</param>
    /// <returns>图标尺寸 (Width, Height)。</returns>
    public static (uint Width, uint Height) GetSize(string iconName)
    {
        return EditorResourceCache.Instance.GetIconSize(iconName);
    }

    /// <summary>
    /// 预加载所有常用图标（启动时调用）。
    /// </summary>
    public static void PreloadAll()
    {
        EditorResourceCache.Instance.PreloadIcons([
            Folder, File, Engine, Scene, Image, Sound, Video,
            Lua, Css, Html, GalStoryScript, Left, Right, Reload
        ]);
    }
}
