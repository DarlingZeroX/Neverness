using Neverness.Editor.Framework.Interface;

namespace Neverness.Editor.Script.Private.Panel;

/// <summary>
/// 脚本控制台面板——显示编译输出、错误和警告。
/// </summary>
internal sealed class ScriptConsolePanel : IEditorPanel
{
    private readonly List<string> _logLines = new();
    private readonly object _lock = new();
    private bool _autoScroll = true;
    private bool _windowOpened = true;

    public string PanelId => "ScriptConsole";
    public string DisplayName => "Script Console";

    // IEditorPanel 实现
    public string GetWindowFullName() => "Script Console";
    public string GetWindowName() => "Script Console";
    public void OpenWindow(bool open) => _windowOpened = open;
    public bool IsWindowOpened() => _windowOpened;

    // IPanel 实现
    public bool IsAsync() => false;

    public void OnUpdate(float delta) { }

    public void OnFixedUpdate() { }

    public void OnGUI()
    {
        // TODO: 使用 ImGui 渲染日志输出
    }

    /// <summary>添加日志行。</summary>
    public void AppendLog(string message)
    {
        lock (_lock)
        {
            _logLines.Add($"[{DateTime.Now:HH:mm:ss}] {message}");

            // 限制日志行数
            if (_logLines.Count > 5000)
            {
                _logLines.RemoveRange(0, _logLines.Count - 5000);
            }
        }
    }

    /// <summary>清除所有日志。</summary>
    public void Clear()
    {
        lock (_lock)
        {
            _logLines.Clear();
        }
    }

    /// <summary>获取当前日志内容（用于渲染）。</summary>
    public IReadOnlyList<string> GetLogs()
    {
        lock (_lock)
        {
            return _logLines.ToArray();
        }
    }

    /// <summary>是否自动滚动到底部。</summary>
    public bool AutoScroll
    {
        get => _autoScroll;
        set => _autoScroll = value;
    }
}
