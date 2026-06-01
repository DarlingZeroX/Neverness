namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本编译状态。
/// </summary>
public enum ScriptCompileStatus
{
    /// <summary>未编译。</summary>
    None,

    /// <summary>编译中。</summary>
    Compiling,

    /// <summary>编译成功。</summary>
    Success,

    /// <summary>编译失败。</summary>
    Error
}

/// <summary>
/// 单个脚本编译诊断信息。
/// </summary>
public sealed class ScriptDiagnostic
{
    public string FilePath { get; init; } = string.Empty;
    public int Line { get; init; }
    public int Column { get; init; }
    public string Message { get; init; } = string.Empty;
    public bool IsError { get; init; }
    public bool IsWarning => !IsError;
}

/// <summary>
/// 脚本编译结果。
/// </summary>
public sealed class ScriptCompileResult
{
    public ScriptCompileStatus Status { get; init; }
    public IReadOnlyList<ScriptDiagnostic> Diagnostics { get; init; } = Array.Empty<ScriptDiagnostic>();
    public TimeSpan Duration { get; init; }

    public bool Success => Status == ScriptCompileStatus.Success;
    public int ErrorCount => Diagnostics.Count(d => d.IsError);
    public int WarningCount => Diagnostics.Count(d => d.IsWarning);
}

/// <summary>
/// 脚本文件变更事件。
/// </summary>
public sealed class ScriptFileChangedEventArgs : EventArgs
{
    public string FilePath { get; init; } = string.Empty;
    public ScriptFileChangeKind ChangeKind { get; init; }
}

/// <summary>
/// 脚本文件变更类型。
/// </summary>
public enum ScriptFileChangeKind
{
    Created,
    Modified,
    Deleted,
    Renamed
}
