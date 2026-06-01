using Neverness.Editor.Core.Public;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本编辑器服务——提供 Gameplay 脚本的编译、热重载、诊断等功能。
/// </summary>
public interface IScriptEditorService
{
    /// <summary>当前编译状态。</summary>
    ScriptCompileStatus Status { get; }

    /// <summary>最后一次编译结果。</summary>
    ScriptCompileResult? LastResult { get; }

    /// <summary>脚本文件变更时触发。</summary>
    event EventHandler<ScriptFileChangedEventArgs>? FileChanged;

    /// <summary>编译完成时触发。</summary>
    event EventHandler<ScriptCompileResult>? CompileFinished;

    /// <summary>异步编译所有脚本。</summary>
    Task<ScriptCompileResult> CompileAllAsync(CancellationToken ct = default);

    /// <summary>异步编译单个脚本文件。</summary>
    Task<ScriptCompileResult> CompileFileAsync(string filePath, CancellationToken ct = default);

    /// <summary>请求热重载（通常在编译成功后自动触发）。</summary>
    Task<bool> RequestHotReloadAsync(CancellationToken ct = default);

    /// <summary>获取指定项目目录下的所有脚本文件路径。</summary>
    IReadOnlyList<string> GetScriptFiles(string projectDirectory);

    /// <summary>开始监听脚本文件变更。</summary>
    void StartWatching(string projectDirectory);

    /// <summary>停止监听脚本文件变更。</summary>
    void StopWatching();
}
