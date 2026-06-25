using AvaloniaEdit;
using AvaloniaEdit.TextMate;
using TextMateSharp.Internal.Types;
using TextMateSharp.Registry;
using TextMateSharp.Themes;

namespace Neverness.Editor.CodeEditor.Private.Syntax;

/// <summary>
/// TextMate 语法加载器——从 VFS 加载 .tmLanguage.json 语法文件。
///
/// VFS 路径：/editor/codeEditor/{language}/syntaxes/{language}.tmLanguage.json
/// 物理路径：Resource/Editor/codeEditor/{language}/syntaxes/{language}.tmLanguage.json
///
/// 支持的语言：html, css, csharp
/// </summary>
public sealed class TextMateGrammarLoader
{
    /// <summary>语法文件 VFS 基础路径。</summary>
    private const string VfsBasePath = "/editor/codeEditor";

    /// <summary>语言 → VFS 语法文件路径映射。</summary>
    private static readonly Dictionary<string, string> LanguageGrammarPaths = new(StringComparer.OrdinalIgnoreCase)
    {
        ["HTML"] = $"{VfsBasePath}/html/syntaxes/html.tmLanguage.json",
        ["CSS"] = $"{VfsBasePath}/css/syntaxes/css.tmLanguage.json",
        ["C#"] = $"{VfsBasePath}/csharp/syntaxes/csharp.tmLanguage.json",
    };

    /// <summary>语言 → 物理路径缓存。</summary>
    private readonly Dictionary<string, string> _physicalPaths = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>已加载的语言名称集合。</summary>
    public IReadOnlyCollection<string> LoadedLanguages => _physicalPaths.Keys;

    /// <summary>
    /// 初始化——解析 VFS 路径到物理路径。
    /// </summary>
    public void Initialize()
    {
        Console.WriteLine($"[TextMateGrammarLoader] 初始化开始");

        foreach (var (language, vfsPath) in LanguageGrammarPaths)
        {
            ResolvePhysicalPath(language, vfsPath);
        }

        // 如果 VFS 解析失败，尝试直接从已知物理路径加载
        if (_physicalPaths.Count == 0)
        {
            LoadFromDiskFallback();
        }

        Console.WriteLine($"[TextMateGrammarLoader] 已解析 {_physicalPaths.Count} 个语法路径: {string.Join(", ", _physicalPaths.Keys)}");
    }

    /// <summary>
    /// 在 TextEditor 上安装 TextMate 语法高亮。
    /// 使用 Installation.SetGrammarFile() 加载自定义语法文件。
    /// </summary>
    public TextMate.Installation Install(TextEditor editor, string language)
    {
        // 使用最小化的 IRegistryOptions（让 TextMateSharp 使用默认行为）
        var locator = new MinimalRegistryOptions();

        // 安装 TextMate
        var installation = editor.InstallTextMate(locator);

        // 如果有自定义语法文件，通过 SetGrammarFile 加载
        if (_physicalPaths.TryGetValue(language, out var physicalPath))
        {
            try
            {
                installation.SetGrammarFile(physicalPath);
                Console.WriteLine($"[TextMateGrammarLoader] 已加载自定义语法: {language} ← {Path.GetFileName(physicalPath)}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[TextMateGrammarLoader] 自定义语法加载失败，使用默认: {language}, {ex.Message}");
            }
        }
        else
        {
            Console.WriteLine($"[TextMateGrammarLoader] 使用默认语法: {language}");
        }

        return installation;
    }

    /// <summary>从 VFS 路径解析物理路径。</summary>
    private void ResolvePhysicalPath(string language, string vfsPath)
    {
        try
        {
            var absPath = Neverness.Runtime.VFS.VFSService.GetAbsolutePath(vfsPath);
            if (!string.IsNullOrEmpty(absPath) && File.Exists(absPath))
            {
                _physicalPaths[language] = absPath;
                Console.WriteLine($"[TextMateGrammarLoader] VFS 解析成功: {language} → {absPath}");
            }
            else
            {
                Console.WriteLine($"[TextMateGrammarLoader] VFS 路径不存在: {vfsPath} → {absPath ?? "null"}");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TextMateGrammarLoader] VFS 解析失败: {language}, {ex.Message}");
        }
    }

    /// <summary>磁盘回退：直接从已知物理路径加载。</summary>
    private void LoadFromDiskFallback()
    {
        var editorRoot = Neverness.Runtime.VFS.ProjectPaths.EditorResource.FullPath;
        Console.WriteLine($"[TextMateGrammarLoader] EditorResource 路径: {editorRoot}");

        var diskPaths = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["HTML"] = Path.Combine(editorRoot, "codeEditor", "html", "syntaxes", "html.tmLanguage.json"),
            ["CSS"] = Path.Combine(editorRoot, "codeEditor", "css", "syntaxes", "css.tmLanguage.json"),
            ["C#"] = Path.Combine(editorRoot, "codeEditor", "csharp", "syntaxes", "csharp.tmLanguage.json"),
        };

        foreach (var (language, filePath) in diskPaths)
        {
            if (File.Exists(filePath))
            {
                _physicalPaths[language] = filePath;
                Console.WriteLine($"[TextMateGrammarLoader] 磁盘发现: {language} ← {filePath}");
            }
            else
            {
                Console.WriteLine($"[TextMateGrammarLoader] 文件不存在: {filePath}");
            }
        }
    }

    /// <summary>最小化 IRegistryOptions 实现——提供 DarkPlus 主题。</summary>
    private sealed class MinimalRegistryOptions : IRegistryOptions
    {
        public ICollection<string> GetInjections(string scopeName) => Array.Empty<string>();
        public IRawTheme GetDefaultTheme() => new DarkPlusTheme();
        public IRawTheme GetTheme(string scopeName) => new DarkPlusTheme();
        public IRawGrammar GetGrammar(string scopeName) => null;
    }

    /// <summary>DarkPlus 风格 IRawTheme 实现。</summary>
    private sealed class DarkPlusTheme : IRawTheme
    {
        public string GetName() => "Dark+";
        public string GetInclude() => null;

        public ICollection<IRawThemeSetting> GetSettings() => new IRawThemeSetting[]
        {
            new ThemeSetting(null, "#D4D4D4", "#1E1E1E"),
        };

        public ICollection<IRawThemeSetting> GetTokenColors() => new IRawThemeSetting[]
        {
            new ThemeSetting("keyword", "#C586C0", null),
            new ThemeSetting("string", "#CE9178", null),
            new ThemeSetting("number", "#B5CEA8", null),
            new ThemeSetting("comment", "#6A9955", null),
            new ThemeSetting("type", "#4EC9B0", null),
            new ThemeSetting("function", "#DCDCAA", null),
            new ThemeSetting("variable", "#9CDCFE", null),
            new ThemeSetting("tag", "#569CD6", null),
            new ThemeSetting("attribute", "#9CDCFE", null),
            new ThemeSetting("punctuation", "#808080", null),
        };

        public ICollection<KeyValuePair<string, object>> GetGuiColors() => new List<KeyValuePair<string, object>>
        {
            new("editor.background", "#1E1E1E"),
            new("editor.foreground", "#D4D4D4"),
        };
    }

    /// <summary>IRawThemeSetting 实现。</summary>
    private sealed class ThemeSetting : IRawThemeSetting
    {
        private readonly string _name;
        private readonly string _foreground;
        private readonly string _background;

        public ThemeSetting(string name, string foreground, string background)
        {
            _name = name;
            _foreground = foreground;
            _background = background;
        }

        public string GetName() => _name;
        public object GetScope() => _name != null ? new[] { _name } : null;
        public IThemeSetting GetSetting() => new ThemeSettingValue(_foreground, _background);
    }

    /// <summary>IThemeSetting 实现。</summary>
    private sealed class ThemeSettingValue : IThemeSetting
    {
        private readonly string _foreground;
        private readonly string _background;

        public ThemeSettingValue(string foreground, string background)
        {
            _foreground = foreground;
            _background = background;
        }

        public object GetFontStyle() => null;
        public string GetBackground() => _background;
        public string GetForeground() => _foreground;
    }
}
