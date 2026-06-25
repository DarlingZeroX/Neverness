using Neverness.Editor.CodeEditor.Private;
using Neverness.Editor.CodeEditor.Private.Syntax;

namespace Neverness.Editor.CodeEditor.Public;

/// <summary>
/// 代码编辑器模块入口——由 EditorApplicationRunner 调用 Install()。
///
/// 职责：
/// 1. 初始化 TextMate 语法加载器
/// 2. 创建并注册 ICodeEditorService
/// 3. 注册资产打开器（CodeEditorAssetOpener）
/// </summary>
public static class CodeEditorModule
{
    private static bool _initialized;
    private static CodeEditorServiceImpl? _serviceImpl;
    private static TextMateGrammarLoader? _grammarLoader;

    /// <summary>模块是否已初始化。</summary>
    public static bool IsInitialized => _initialized;

    /// <summary>服务实现实例（供 AssetOpener 和 Dock 集成使用）。</summary>
    public static CodeEditorServiceImpl? ServiceImpl => _serviceImpl;

    /// <summary>语法加载器实例。</summary>
    public static TextMateGrammarLoader? GrammarLoader => _grammarLoader;

    /// <summary>安装代码编辑器模块。</summary>
    public static void Install()
    {
        if (_initialized)
        {
            Console.WriteLine("[CodeEditorModule] 已初始化，跳过重复调用。");
            return;
        }

        // 1. 初始化 TextMate 语法加载器
        _grammarLoader = new TextMateGrammarLoader();
        _grammarLoader.Initialize();

        // 2. 创建服务实现
        _serviceImpl = new CodeEditorServiceImpl();

        // 3. 注册到服务定位器（供其他模块查询）
        var context = Neverness.Editor.Core.Public.EditorCoreModule.Context;
        context.RegisterService<ICodeEditorService>(_serviceImpl);

        _initialized = true;
        Console.WriteLine("[CodeEditorModule] 代码编辑器模块已初始化。");
    }
}
