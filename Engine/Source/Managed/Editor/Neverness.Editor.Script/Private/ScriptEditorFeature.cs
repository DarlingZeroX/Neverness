using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Script.Public;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本编辑器 Feature——向 Core 注册脚本相关面板、菜单和服务。
/// </summary>
internal sealed class ScriptEditorFeature : IEditorFeature
{
    private ScriptEditorServiceImpl? _service;

    public string FeatureId => "com.neverness.script";
    public string DisplayName => "Script Editor";
    public IReadOnlyList<string> Dependencies => new[] { "com.neverness.core" };

    /// <summary>供 ScriptEditorModule 访问服务实例。</summary>
    internal IScriptEditorService EditorService =>
        _service ?? throw new InvalidOperationException("ScriptEditorFeature is not initialized.");

    public void Initialize(IEditorContext context)
    {
        _service = new ScriptEditorServiceImpl();

        // 注册脚本服务到编辑器上下文
        context.RegisterService<IScriptEditorService>(_service);

        // 注册脚本控制台面板
        context.Panels.AddChildPanel("Script Console", new Panel.ScriptConsolePanel());

        // 注册菜单命令
        var compileAllCmd = new EditorCommand
        {
            Id = "script.compile_all",
            DisplayName = "Compile All Scripts",
            Execute = _ => _service?.CompileAllAsync(),
            Tooltip = "编译所有 Gameplay 脚本"
        };

        var hotReloadCmd = new EditorCommand
        {
            Id = "script.hot_reload",
            DisplayName = "Hot Reload",
            Execute = _ => _service?.RequestHotReloadAsync(),
            Tooltip = "请求热重载脚本"
        };

        context.Menus.RegisterCommand(compileAllCmd);
        context.Menus.RegisterCommand(hotReloadCmd);

        context.Menus.Register(new EditorMenuItem
        {
            Path = "Scripts/Compile All Scripts",
            Command = compileAllCmd,
            Shortcut = "F7",
            SortOrder = 100
        });

        context.Menus.Register(new EditorMenuItem
        {
            Path = "Scripts/Hot Reload",
            Command = hotReloadCmd,
            Shortcut = "Ctrl+Shift+F5",
            SortOrder = 200
        });
    }

    public void Shutdown(IEditorContext context)
    {
        _service?.Dispose();
        _service = null;
    }
}
