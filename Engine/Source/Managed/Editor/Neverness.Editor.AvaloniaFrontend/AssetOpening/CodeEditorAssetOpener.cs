using Avalonia.Threading;
using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.AvaloniaFrontend.Dock;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.CodeEditor.Private;
using Neverness.Editor.CodeEditor.Private.Syntax;
using Neverness.Editor.CodeEditor.Private.Views;
using Neverness.Editor.CodeEditor.Public;
using Neverness.Editor.Settings;
using Neverness.Editor.Settings.Private.Descriptors;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.AvaloniaFrontend.AssetOpening;

/// <summary>
/// 代码资产打开器——在编辑器内以 Dock 标签页打开代码文件。
///
/// 支持的 TypeId：
/// - 12 (HtmlDocument): .html, .htm, .rml, .css, .rcss, .js
/// - 5 (Shader): .hlsl, .glsl
/// - 11 (CSharpScript): .cs（仅当 EditorPreferences.CsEditorMode == InlineEditor 时）
///
/// 注册方式：在 AvaloniaFrontendModule.RegisterAssetOpeners() 中手动注册。
/// </summary>
[AssetOpener(AssetTypeId.HtmlDocument)]
public sealed class CodeEditorAssetOpener : IAssetOpener
{
    private readonly AssetEditorManager _editorManager;
    private readonly CodeEditorServiceImpl _serviceImpl;
    private readonly TextMateGrammarLoader _grammarLoader;

    public CodeEditorAssetOpener(
        AssetEditorManager editorManager,
        CodeEditorServiceImpl serviceImpl,
        TextMateGrammarLoader grammarLoader)
    {
        _editorManager = editorManager;
        _serviceImpl = serviceImpl;
        _grammarLoader = grammarLoader;
    }

    /// <summary>支持的扩展名集合。</summary>
    private static readonly HashSet<string> SupportedExtensions = new(StringComparer.OrdinalIgnoreCase)
    {
        ".html", ".htm", ".rml", ".css", ".rcss", ".js",
        ".hlsl", ".glsl", ".shader",
    };

    public bool CanOpen(ulong assetTypeId)
    {
        // TypeId 12 (HtmlDocument) 和 5 (Shader) 直接支持
        if (assetTypeId is AssetTypeId.HtmlDocument or AssetTypeId.Shader)
            return true;

        // TypeId 11 (CSharpScript) 需要检查设置
        if (assetTypeId == AssetTypeId.CSharpScript)
            return EditorSettings.Preferences.CsEditorMode == CsEditorMode.InlineEditor;

        return false;
    }

    public async Task OpenAsync(AssetOpenContext context)
    {
        var vfsPath = context.VirtualPath.FullPath;
        var fileName = context.VirtualPath.FileName;
        var ext = Path.GetExtension(vfsPath).ToLowerInvariant();

        // 额外检查：扩展名是否在支持列表中
        if (!SupportedExtensions.Contains(ext))
        {
            Console.WriteLine($"[CodeEditorAssetOpener] 不支持的扩展名: {ext}");
            return;
        }

        Console.WriteLine($"[CodeEditorAssetOpener] 打开代码文件: {fileName} ({vfsPath})");

        // 如果已在编辑器中打开，通过 Dock 激活现有标签
        if (_serviceImpl.IsFileOpen(vfsPath))
        {
            Console.WriteLine($"[CodeEditorAssetOpener] 文件已打开，激活: {vfsPath}");
            var existingId = DockableAssetEditorFramework.ToEditorId(context.Guid);
            var framework = Public.AvaloniaFrontendModule.CreateAssetEditorFramework(_editorManager);
            await Dispatcher.UIThread.InvokeAsync(() =>
                framework.TryActivateExisting(context.Guid));
            return;
        }

        // 从 VFS 读取文件内容，VFS 失败则回退到磁盘直接读取
        string text;
        try
        {
            text = VFSService.ReadText(vfsPath) ?? "";
            Console.WriteLine($"[CodeEditorAssetOpener] VFS 读取: path={vfsPath}, textLength={text.Length}");

            // VFS 返回空且文件路径可解析时，回退磁盘读取
            if (text.Length == 0)
            {
                var absPath = VFSService.GetAbsolutePath(vfsPath);
                if (!string.IsNullOrEmpty(absPath) && File.Exists(absPath))
                {
                    text = File.ReadAllText(absPath);
                    Console.WriteLine($"[CodeEditorAssetOpener] 磁盘回退读取: absPath={absPath}, textLength={text.Length}");
                }
                else
                {
                    Console.WriteLine($"[CodeEditorAssetOpener] VFS 和磁盘均无内容: vfsPath={vfsPath}, absPath={absPath ?? "null"}");
                }
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[CodeEditorAssetOpener] 读取文件失败: {vfsPath}, {ex.Message}");
            return;
        }

        // 在 UI 线程创建编辑器视图并打开 Dock 标签
        await Dispatcher.UIThread.InvokeAsync(() =>
        {
            var view = new CodeEditorView();
            view.Initialize(vfsPath, fileName, text);

            // 安装 TextMate 语法高亮
            var language = CodeEditorView.DetectLanguage(vfsPath);
            try
            {
                _grammarLoader.Install(view.TextEditor, language);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[CodeEditorAssetOpener] TextMate 安装失败: {ex.Message}");
            }

            // 注册到服务
            _serviceImpl.OpenFile(vfsPath, fileName);
            _serviceImpl.RegisterView(vfsPath, view);

            // 设置回调
            view.OnSaveRequested = path => _serviceImpl.SaveFile(path);
            view.OnDirtyStateChanged = (path, dirty) => _serviceImpl.MarkDirty(path);

            // 通过 Dock 系统打开标签页
            var editorFramework = Public.AvaloniaFrontendModule.CreateAssetEditorFramework(_editorManager);
            editorFramework.OpenCodeEditor(fileName, context.Guid, view);
        });

        Console.WriteLine($"[CodeEditorAssetOpener] 代码编辑器已打开: {fileName}");
    }

    /// <summary>尝试安装 TextMate 语法高亮。失败时静默降级（无语法高亮）。</summary>
    private static void TryInstallTextMate(CodeEditorView view)
    {
        try
        {
            // 通过反射安全地尝试安装 TextMateSharp
            // 避免硬依赖 TextMateSharp，如果不可用则降级为纯文本
            var registryOptionsType = Type.GetType("TextMateSharp.RegistryOptions, TextMateSharp");
            var themeNameType = Type.GetType("TextMateSharp.ThemeName, TextMateSharp");

            if (registryOptionsType == null || themeNameType == null)
            {
                Console.WriteLine("[CodeEditorAssetOpener] TextMateSharp 不可用，使用纯文本模式");
                return;
            }

            // 创建 RegistryOptions(ThemeName.DarkPlus)
            var darkPlus = Enum.Parse(themeNameType, "DarkPlus");
            var registryOptions = Activator.CreateInstance(registryOptionsType, darkPlus);
            if (registryOptions == null) return;

            // 查找 InstallTextMate 扩展方法
            var installMethod = typeof(AvaloniaEdit.TextEditor).GetMethod("InstallTextMate");
            if (installMethod != null)
            {
                installMethod.Invoke(view.TextEditor, new[] { registryOptions });
                Console.WriteLine("[CodeEditorAssetOpener] TextMate 语法高亮已安装");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[CodeEditorAssetOpener] TextMate 安装失败（降级为纯文本）: {ex.Message}");
        }
    }
}
