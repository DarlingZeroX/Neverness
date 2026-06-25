using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using AvaloniaEdit;

namespace Neverness.Editor.CodeEditor.Private.Views;

/// <summary>
/// 代码编辑器视图——封装 AvaloniaEdit TextEditor 控件。
/// 纯代码构建 UI（与 TextureViewerControl 模式一致），避免 AXAML 加载问题。
/// </summary>
public class CodeEditorView : UserControl
{
    private readonly TextEditor _editor;
    private readonly TextBlock _fileNameText;
    private readonly TextBlock _dirtyIndicator;
    private readonly TextBlock _cursorInfo;
    private readonly TextBlock _languageText;

    private string _vfsPath = "";
    private string _fileName = "";
    private bool _isDirty;

    /// <summary>VFS 路径（标识此编辑器实例）。</summary>
    public string VfsPath => _vfsPath;

    /// <summary>文件名。</summary>
    public string FileName => _fileName;

    /// <summary>是否有未保存改动。</summary>
    public bool IsDirty => _isDirty;

    /// <summary>底层 TextEditor 控件（供外部安装 TextMate 语法）。</summary>
    public TextEditor TextEditor => _editor;

    /// <summary>脏状态变更回调（由 CodeEditorServiceImpl 设置）。</summary>
    public Action<string, bool>? OnDirtyStateChanged { get; set; }

    /// <summary>保存回调（由 CodeEditorServiceImpl 设置）。</summary>
    public Func<string, bool>? OnSaveRequested { get; set; }

    public CodeEditorView()
    {
        // ── 工具栏 ──
        _fileNameText = new TextBlock
        {
            VerticalAlignment = VerticalAlignment.Center,
            FontWeight = FontWeight.SemiBold,
            FontSize = 13,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };

        _dirtyIndicator = new TextBlock
        {
            Text = "*",
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = new SolidColorBrush(Color.Parse("#FFFFC107")),
            IsVisible = false,
            FontSize = 16,
            Margin = new Thickness(4, 0, 0, 0),
        };

        var languageLabel = new TextBlock
        {
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = new SolidColorBrush(Color.Parse("#FF858585")),
            FontSize = 12,
        };
        _languageText = languageLabel;

        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 8,
            Margin = new Thickness(8, 4),
            Children = { _fileNameText, _dirtyIndicator, languageLabel }
        };

        var toolbarBorder = new Border
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            BorderBrush = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            BorderThickness = new Thickness(0, 0, 0, 1),
            Child = toolbar,
        };
        DockPanel.SetDock(toolbarBorder, Dock.Top);

        // ── 状态栏 ──
        _cursorInfo = new TextBlock
        {
            Text = "行 1, 列 1",
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };

        var statusBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 16,
            Margin = new Thickness(8, 2),
            Children =
            {
                _cursorInfo,
                new TextBlock { Text = "UTF-8", FontSize = 12, Foreground = new SolidColorBrush(Color.Parse("#FF858585")) },
                new TextBlock { Text = "CRLF", FontSize = 12, Foreground = new SolidColorBrush(Color.Parse("#FF858585")) },
            }
        };

        var statusBarBorder = new Border
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            BorderBrush = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            BorderThickness = new Thickness(0, 1, 0, 0),
            Child = statusBar,
        };
        DockPanel.SetDock(statusBarBorder, Dock.Bottom);

        // ── AvaloniaEdit 主体 ──
        _editor = new TextEditor
        {
            ShowLineNumbers = true,
            FontFamily = new FontFamily("Cascadia Code,Consolas,monospace"),
            FontSize = 14,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
        };

        _editor.TextChanged += (_, _) => SetDirty(true);
        _editor.TextArea.Caret.PositionChanged += (_, _) => UpdateCursorInfo();

        // Ctrl+S 快捷键
        KeyBindings.Add(new KeyBinding
        {
            Gesture = new KeyGesture(Key.S, KeyModifiers.Control),
            Command = new SaveCommand(this),
        });

        // ── 组装 ──
        var root = new DockPanel
        {
            Children = { toolbarBorder, statusBarBorder, _editor }
        };

        Content = root;
    }

    /// <summary>初始化编辑器内容。</summary>
    public void Initialize(string vfsPath, string fileName, string text)
    {
        _vfsPath = vfsPath;
        _fileName = fileName;

        _fileNameText.Text = fileName;
        _editor.Text = text ?? "";
        _languageText.Text = DetectLanguage(vfsPath);

        UpdateCursorInfo();
        SetDirty(false);

        Console.WriteLine($"[CodeEditorView] Initialize: file={fileName}, textLength={(_editor.Text?.Length ?? -1)}");
    }

    /// <summary>获取当前编辑器文本内容。</summary>
    public string GetText() => _editor.Text ?? "";

    /// <summary>设置编辑器文本内容。</summary>
    public void SetText(string text)
    {
        _editor.Text = text;
        SetDirty(false);
    }

    /// <summary>执行保存操作。</summary>
    public void Save()
    {
        if (!_isDirty) return;
        if (OnSaveRequested?.Invoke(_vfsPath) == true)
            SetDirty(false);
    }

    private void SetDirty(bool dirty)
    {
        if (_isDirty == dirty) return;
        _isDirty = dirty;
        _dirtyIndicator.IsVisible = dirty;
        _fileNameText.Text = dirty ? $"{_fileName} *" : _fileName;
        OnDirtyStateChanged?.Invoke(_vfsPath, dirty);
    }

    private void UpdateCursorInfo()
    {
        _cursorInfo.Text = $"行 {_editor.TextArea.Caret.Line}, 列 {_editor.TextArea.Caret.Column}";
    }

    public static string DetectLanguage(string vfsPath)
    {
        var ext = Path.GetExtension(vfsPath).ToLowerInvariant();
        return ext switch
        {
            ".html" or ".htm" or ".rml" => "HTML",
            ".css" or ".rcss" => "CSS",
            ".js" => "JavaScript",
            ".cs" => "C#",
            ".hlsl" or ".glsl" or ".shader" => "GLSL",
            ".json" => "JSON",
            ".xml" or ".xaml" or ".axaml" => "XML",
            ".md" => "Markdown",
            _ => "Plain Text"
        };
    }

    private sealed class SaveCommand : System.Windows.Input.ICommand
    {
        private readonly CodeEditorView _owner;
        public SaveCommand(CodeEditorView owner) => _owner = owner;
        public event EventHandler? CanExecuteChanged;
        public bool CanExecute(object? parameter) => _owner._isDirty;
        public void Execute(object? parameter) => _owner.Save();
    }
}
