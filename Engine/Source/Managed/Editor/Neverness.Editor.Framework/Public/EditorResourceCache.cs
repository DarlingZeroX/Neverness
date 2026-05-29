using System.Collections.Concurrent;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS.Public;
using StbImageSharp;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 编辑器内置资源缓存——统一管理图标纹理和本地化文本的加载与缓存。
///
/// 虚拟路径：/editor/（对应 ProjectPaths.EditorResource）
/// 物理路径：E:\Neverness\Resource\Editor
///
/// 设计原则：
/// 1. 纹理按需加载，缓存 ImGui 纹理句柄，生命周期跟随编辑器
/// 2. 本地化文本启动时预加载，支持 Key=Value 格式
/// 3. 所有读取通过 VFS，不直接访问物理路径
/// 4. 线程安全：纹理使用 ConcurrentDictionary，文本加载在主线程
/// </summary>
public sealed class EditorResourceCache
{
    // ── 单例 ──
    private static EditorResourceCache? _instance;

    /// <summary>全局单例。</summary>
    public static EditorResourceCache Instance => _instance ??= new EditorResourceCache();

    // ── 纹理缓存 ──

    /// <summary>纹理缓存：虚拟路径 → ImGui 纹理句柄。</summary>
    private readonly ConcurrentDictionary<string, ulong> _textureCache = new();

    /// <summary>纹理尺寸缓存：虚拟路径 → (Width, Height)。</summary>
    private readonly ConcurrentDictionary<string, (uint Width, uint Height)> _textureSizeCache = new();

    /// <summary>已加载的纹理 key 集合（用于释放）。</summary>
    private readonly ConcurrentDictionary<string, ulong> _textureKeys = new();

    // ── 本地化缓存 ──

    /// <summary>当前语言的本地化文本：Key → Value。</summary>
    private readonly Dictionary<string, string> _localization = new();

    /// <summary>当前语言代码。</summary>
    private string _currentLanguage = "ZH_CN";

    /// <summary>本地化是否已加载。</summary>
    private bool _localizationLoaded;

    // ── 常量 ──

    /// <summary>编辑器资源 VFS 前缀。</summary>
    public const string VfsPrefix = "/editor/";

    /// <summary>图标子目录。</summary>
    public const string IconsDir = "/editor/icons/";

    /// <summary>本地化子目录。</summary>
    public const string LocalizationDir = "/editor/localization/";

    private EditorResourceCache() { }

    // ── 纹理 API ──

    /// <summary>
    /// 获取图标纹理的 ImGui 句柄。
    /// 首次调用时加载并缓存，后续直接返回缓存。
    /// </summary>
    /// <param name="iconName">图标文件名（如 "folder.png"、"file.png"）。</param>
    /// <returns>ImGui 纹理句柄，0 = 加载失败。</returns>
    public ulong GetIconTexture(string iconName)
    {
        if (string.IsNullOrEmpty(iconName))
            return 0;

        string vfsPath = IconsDir + iconName;

        // 已缓存，直接返回
        if (_textureCache.TryGetValue(vfsPath, out var cachedHandle))
            return cachedHandle;

        // 加载纹理
        return LoadAndCacheTexture(vfsPath);
    }

    /// <summary>
    /// 获取指定 VFS 路径的纹理 ImGui 句柄。
    /// </summary>
    /// <param name="vfsPath">完整 VFS 路径（如 "/editor/icons/folder.png"）。</param>
    /// <returns>ImGui 纹理句柄，0 = 加载失败。</returns>
    public ulong GetTexture(string vfsPath)
    {
        if (string.IsNullOrEmpty(vfsPath))
            return 0;

        if (_textureCache.TryGetValue(vfsPath, out var cachedHandle))
            return cachedHandle;

        return LoadAndCacheTexture(vfsPath);
    }

    /// <summary>
    /// 获取纹理尺寸。
    /// </summary>
    /// <param name="iconName">图标文件名。</param>
    /// <returns>纹理尺寸，(0,0) = 未加载。</returns>
    public (uint Width, uint Height) GetIconSize(string iconName)
    {
        string vfsPath = IconsDir + iconName;
        return _textureSizeCache.TryGetValue(vfsPath, out var size) ? size : (0, 0);
    }

    /// <summary>
    /// 预加载指定图标列表（启动时调用，避免首次使用时的加载延迟）。
    /// </summary>
    /// <param name="iconNames">图标文件名列表。</param>
    public void PreloadIcons(IEnumerable<string> iconNames)
    {
        foreach (var name in iconNames)
        {
            _ = GetIconTexture(name);
        }
    }

    // ── 本地化 API ──

    /// <summary>
    /// 获取本地化文本。
    /// </summary>
    /// <param name="key">文本 Key（如 "Open"、"Cancel"）。</param>
    /// <returns>本地化文本，如果未找到则返回 key 本身。</returns>
    public string GetText(string key)
    {
        if (!_localizationLoaded)
            LoadLocalization(_currentLanguage);

        return _localization.TryGetValue(key, out var value) ? value : key;
    }

    /// <summary>
    /// 获取本地化文本（支持格式化参数）。
    /// </summary>
    /// <param name="key">文本 Key（如 "The texture is loaded successfully: %s [Width: %d  Height: %d]"）。</param>
    /// <param name="args">格式化参数。</param>
    /// <returns>格式化后的本地化文本。</returns>
    public string GetText(string key, params object[] args)
    {
        string template = GetText(key);
        try
        {
            return string.Format(template, args);
        }
        catch
        {
            return template;
        }
    }

    /// <summary>
    /// 切换语言。
    /// </summary>
    /// <param name="languageCode">语言代码（如 "ZH_CN"、"EN_US"）。</param>
    public void SetLanguage(string languageCode)
    {
        if (_currentLanguage == languageCode && _localizationLoaded)
            return;

        _currentLanguage = languageCode;
        _localizationLoaded = false;
        _localization.Clear();
    }

    /// <summary>
    /// 获取当前语言代码。
    /// </summary>
    public string CurrentLanguage => _currentLanguage;

    /// <summary>
    /// 预加载本地化文本（启动时调用）。
    /// </summary>
    public void PreloadLocalization(string? languageCode = null)
    {
        LoadLocalization(languageCode ?? _currentLanguage);
    }

    // ── 内部实现 ──

    /// <summary>
    /// 加载纹理并缓存。
    /// </summary>
    private ulong LoadAndCacheTexture(string vfsPath)
    {
        try
        {
            // 通过 VFS 读取纹理文件
            var fileData = VFS.ReadBytes(vfsPath);
            if (fileData == null || fileData.Length == 0)
            {
                Console.WriteLine($"[EditorResourceCache] 纹理加载失败: {vfsPath}");
                return 0;
            }

            // 解码为 RGBA8
            var (width, height, pixelData) = DecodePngToRgba8(fileData);
            if (pixelData == null || width == 0 || height == 0)
            {
                Console.WriteLine($"[EditorResourceCache] 纹理解码失败: {vfsPath}");
                return 0;
            }

            // 创建 GPU 纹理
            var textureKey = TextureInterop.CreateTexture(
                (uint)width, (uint)height,
                pixelData.AsSpan(),
                isSRGB: true);

            if (textureKey == 0)
            {
                Console.WriteLine($"[EditorResourceCache] GPU 纹理创建失败: {vfsPath}");
                return 0;
            }

            // 获取 ImGui 纹理句柄
            var imGuiHandle = TextureInterop.GetImGuiTextureHandle(textureKey);

            // 缓存
            _textureCache[vfsPath] = imGuiHandle;
            _textureKeys[vfsPath] = textureKey;
            _textureSizeCache[vfsPath] = ((uint)width, (uint)height);

            Console.WriteLine($"[EditorResourceCache] 纹理已加载: {vfsPath} ({width}x{height})");
            return imGuiHandle;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorResourceCache] 纹理加载异常: {vfsPath} - {ex.Message}");
            return 0;
        }
    }

    /// <summary>
    /// 加载本地化文本文件。
    /// </summary>
    private void LoadLocalization(string languageCode)
    {
        string vfsPath = LocalizationDir + languageCode + ".txt";

        try
        {
            var text = VFS.ReadText(vfsPath);
            if (text == null)
            {
                Console.WriteLine($"[EditorResourceCache] 本地化文件加载失败: {vfsPath}");
                _localizationLoaded = true;
                return;
            }

            ParseLocalization(text);
            _localizationLoaded = true;
            Console.WriteLine($"[EditorResourceCache] 本地化已加载: {vfsPath} ({_localization.Count} 条)");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorResourceCache] 本地化加载异常: {vfsPath} - {ex.Message}");
            _localizationLoaded = true;
        }
    }

    /// <summary>
    /// 解析本地化文本（Key=Value 格式，每行一条）。
    /// </summary>
    private void ParseLocalization(string text)
    {
        _localization.Clear();

        var lines = text.Split('\n');
        foreach (var rawLine in lines)
        {
            var line = rawLine.Trim();
            if (string.IsNullOrEmpty(line) || line.StartsWith('#'))
                continue;

            int eqIndex = line.IndexOf('=');
            if (eqIndex <= 0)
                continue;

            string key = line[..eqIndex].Trim();
            string value = line[(eqIndex + 1)..].Trim();

            if (!string.IsNullOrEmpty(key))
            {
                _localization[key] = value;
            }
        }
    }

    /// <summary>
    /// 使用 StbImageSharp 解码 PNG/JPG 为 RGBA8。
    /// </summary>
    private static (int width, int height, byte[]? data) DecodePngToRgba8(byte[] raw)
    {
        if (raw == null || raw.Length < 8)
            return (0, 0, null);

        try
        {
            using var stream = new MemoryStream(raw);
            var image = ImageResult.FromStream(stream, ColorComponents.RedGreenBlueAlpha);

            if (image == null || image.Data == null)
                return (0, 0, null);

            return (image.Width, image.Height, image.Data);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorResourceCache] 图像解码异常: {ex.Message}");
            return (0, 0, null);
        }
    }

    // ── 清理 ──

    /// <summary>
    /// 释放所有缓存的纹理资源（编辑器关闭时调用）。
    /// </summary>
    public void Dispose()
    {
        foreach (var kvp in _textureKeys)
        {
            TextureInterop.ReleaseTexture(kvp.Value);
        }

        _textureCache.Clear();
        _textureKeys.Clear();
        _textureSizeCache.Clear();
        _localization.Clear();
        _localizationLoaded = false;

        _instance = null;
    }
}
