using Neverness.Editor.Core.Public;
using Neverness.Editor.Script.Public;

namespace Neverness.Editor.Script.Private.Features;

/// <summary>
/// 脚本热重载 Feature——监听文件变更并触发热重载。
/// </summary>
internal sealed class ScriptHotReloadFeature : IEditorFeature
{
    private IScriptEditorService? _scriptService;

    public string FeatureId => "com.neverness.script.hotreload";
    public string DisplayName => "Script Hot Reload";
    public IReadOnlyList<string> Dependencies => new[] { "com.neverness.script" };

    public void Initialize(IEditorContext context)
    {
        if (context.TryGetService<IScriptEditorService>(out var service))
        {
            _scriptService = service;
            _scriptService.FileChanged += OnFileChanged;
        }
    }

    public void Shutdown(IEditorContext context)
    {
        if (_scriptService != null)
        {
            _scriptService.FileChanged -= OnFileChanged;
            _scriptService = null;
        }
    }

    private async void OnFileChanged(object? sender, ScriptFileChangedEventArgs e)
    {
        if (e.ChangeKind == ScriptFileChangeKind.Deleted)
            return;

        if (_scriptService != null)
        {
            // 文件变更后自动编译
            var result = await _scriptService.CompileFileAsync(e.FilePath);

            // 编译成功后请求热重载
            if (result.Success)
            {
                await _scriptService.RequestHotReloadAsync();
            }
        }
    }
}
