using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// 控制台 ViewModel——管理日志条目和过滤状态。
/// Phase 2 验证用 ViewModel。
/// </summary>
public class ConsolePanelViewModel : ViewModelBase
{
    private readonly List<LogEntry> _entries = new();
    private string _filterText = "";
    private LogLevel _filterLevel = LogLevel.All;
    private bool _autoScroll = true;

    /// <summary>日志条目列表（只读视图）。</summary>
    public IReadOnlyList<LogEntry> Entries => _entries;

    /// <summary>过滤文本。</summary>
    public string FilterText
    {
        get => _filterText;
        set => SetProperty(ref _filterText, value);
    }

    /// <summary>过滤级别。</summary>
    public LogLevel FilterLevel
    {
        get => _filterLevel;
        set => SetProperty(ref _filterLevel, value);
    }

    /// <summary>是否自动滚动到底部。</summary>
    public bool AutoScroll
    {
        get => _autoScroll;
        set => SetProperty(ref _autoScroll, value);
    }

    /// <summary>过滤后的条目（View 层读取）。</summary>
    public IEnumerable<LogEntry> FilteredEntries =>
        _entries.Where(e => MatchesFilter(e));

    /// <summary>添加日志条目。</summary>
    public void AddEntry(LogEntry entry)
    {
        _entries.Add(entry);
        OnPropertyChanged(nameof(Entries));
        OnPropertyChanged(nameof(FilteredEntries));
    }

    /// <summary>清空日志。</summary>
    public void Clear()
    {
        _entries.Clear();
        OnPropertyChanged(nameof(Entries));
        OnPropertyChanged(nameof(FilteredEntries));
    }

    private bool MatchesFilter(LogEntry entry)
    {
        if ((entry.Level & _filterLevel) == 0) return false;
        if (!string.IsNullOrEmpty(_filterText) &&
            !entry.Message.Contains(_filterText, StringComparison.OrdinalIgnoreCase))
            return false;
        return true;
    }
}

/// <summary>日志条目。</summary>
public record LogEntry(string Message, LogLevel Level, DateTime Timestamp, string Category = "");

/// <summary>日志级别。</summary>
[Flags]
public enum LogLevel
{
    All = Debug | Info | Warning | Error | Fatal,
    Debug = 1,
    Info = 2,
    Warning = 4,
    Error = 8,
    Fatal = 16
}
