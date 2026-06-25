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

    /// <summary>从 VFS 读取语法文件，写入临时文件供 TextMateSharp 使用。</summary>
    private void ResolvePhysicalPath(string language, string vfsPath)
    {
        try
        {
            var content = Neverness.Runtime.VFS.VFSService.ReadText(vfsPath);
            if (string.IsNullOrEmpty(content))
            {
                Console.WriteLine($"[TextMateGrammarLoader] VFS 读取为空: {vfsPath}");
                return;
            }

            // 写入临时文件（TextMateSharp.SetGrammarFile 需要物理路径）
            var tempDir = Path.Combine(Path.GetTempPath(), "Neverness", "TextMate");
            Directory.CreateDirectory(tempDir);
            var tempPath = Path.Combine(tempDir, $"{language}.tmLanguage.json");
            File.WriteAllText(tempPath, content);
            _physicalPaths[language] = tempPath;
            Console.WriteLine($"[TextMateGrammarLoader] VFS 加载成功: {language} ({content.Length} bytes) → {tempPath}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TextMateGrammarLoader] VFS 加载失败: {language}, {ex.Message}");
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
            // ── 通用 ──
            new ThemeSetting("keyword", "#C586C0"),
            new ThemeSetting("keyword.control", "#C586C0"),
            new ThemeSetting("string", "#CE9178"),
            new ThemeSetting("number", "#B5CEA8"),
            new ThemeSetting("comment", "#6A9955"),
            new ThemeSetting("type", "#4EC9B0"),
            new ThemeSetting("function", "#DCDCAA"),
            new ThemeSetting("variable", "#9CDCFE"),
            new ThemeSetting("constant", "#569CD6"),
            new ThemeSetting("operator", "#D4D4D4"),

            // ── HTML 标签 ──
            new ThemeSetting("entity.name.tag", "#569CD6"),
            new ThemeSetting("entity.other.attribute-name", "#9CDCFE"),
            new ThemeSetting("punctuation.definition.tag", "#808080"),
            new ThemeSetting("punctuation.definition.tag.begin", "#808080"),
            new ThemeSetting("punctuation.definition.tag.end", "#808080"),
            new ThemeSetting("meta.tag", "#D4D4D4"),
            new ThemeSetting("meta.tag.html", "#D4D4D4"),
            new ThemeSetting("meta.tag.block.any", "#D4D4D4"),
            new ThemeSetting("meta.tag.inline.any", "#D4D4D4"),

            // ── HTML 属性 ──
            new ThemeSetting("entity.other.attribute-name.html", "#9CDCFE"),
            new ThemeSetting("string.quoted.double.html", "#CE9178"),
            new ThemeSetting("string.quoted.single.html", "#CE9178"),

            // ── CSS ──
            new ThemeSetting("entity.name.tag.css", "#569CD6"),
            new ThemeSetting("support.type.property-name.css", "#9CDCFE"),
            new ThemeSetting("support.constant.property-value.css", "#CE9178"),
            new ThemeSetting("keyword.control.at-rule.css", "#C586C0"),
            new ThemeSetting("entity.other.attribute-name.class.css", "#DCDCAA"),
            new ThemeSetting("entity.other.attribute-name.id.css", "#DCDCAA"),

            // ── C# ──
            new ThemeSetting("keyword.type.cs", "#569CD6"),
            new ThemeSetting("entity.name.type.cs", "#4EC9B0"),
            new ThemeSetting("variable.other.readwrite.cs", "#9CDCFE"),
            new ThemeSetting("entity.name.function.cs", "#DCDCAA"),
            new ThemeSetting("string.quoted.double.cs", "#CE9178"),
            new ThemeSetting("string.quoted.single.cs", "#CE9178"),
            new ThemeSetting("comment.line.double-slash.cs", "#6A9955"),
            new ThemeSetting("comment.block.cs", "#6A9955"),
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

        public ThemeSetting(string name, string foreground, string background = null)
        {
            _name = name;
            _foreground = foreground;
            _background = background;
        }

        public string GetName() => _name;
        public object GetScope() => _name;
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
