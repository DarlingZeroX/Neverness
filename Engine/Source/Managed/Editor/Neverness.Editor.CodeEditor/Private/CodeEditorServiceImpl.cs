using Neverness.Editor.CodeEditor.Public;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.CodeEditor.Private;

/// <summary>
/// 代码编辑器服务实现——管理打开的文件状态。
/// </summary>
public sealed class CodeEditorServiceImpl : ICodeEditorService
{
    /// <summary>已打开文件条目。</summary>
    internal sealed class OpenFileEntry
    {
        public required string VfsPath { get; init; }
        public required string FileName { get; init; }
        public bool IsDirty { get; set; }
        public Views.CodeEditorView? View { get; set; }
    }

    /// <summary>已打开文件映射（VFS 路径 → 条目）。</summary>
    private readonly Dictionary<string, OpenFileEntry> _openFiles = new(StringComparer.Ordinal);

    /// <summary>当前活动文件的 VFS 路径。</summary>
    private string? _activeFilePath;

    /// <summary>文件打开事件（供 Dock 集成监听）。</summary>
    public event Action<string, string>? FileOpened;

    /// <summary>文件保存事件。</summary>
    public event Action<string>? FileSaved;

    /// <summary>文件关闭事件。</summary>
    public event Action<string>? FileClosed;

    /// <summary>脏状态变更事件。</summary>
    public event Action<string, bool>? DirtyStateChanged;

    public bool OpenFile(string vfsPath, string fileName)
    {
        if (_openFiles.ContainsKey(vfsPath))
        {
            // 已打开，激活现有标签
            _activeFilePath = vfsPath;
            return true;
        }

        var entry = new OpenFileEntry
        {
            VfsPath = vfsPath,
            FileName = fileName,
            IsDirty = false,
        };

        _openFiles[vfsPath] = entry;
        _activeFilePath = vfsPath;

        // 通知外部（Dock 集成会创建标签页）
        FileOpened?.Invoke(vfsPath, fileName);
        return true;
    }

    public bool SaveFile(string vfsPath)
    {
        if (!_openFiles.TryGetValue(vfsPath, out var entry))
            return false;

        if (entry.View == null)
            return false;

        try
        {
            var text = entry.View.GetText();
            VFSService.WriteText(vfsPath, text);
            entry.IsDirty = false;
            DirtyStateChanged?.Invoke(vfsPath, false);
            FileSaved?.Invoke(vfsPath);
            Console.WriteLine($"[CodeEditor] 已保存: {vfsPath}");
            return true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[CodeEditor] 保存失败: {vfsPath}, {ex.Message}");
            return false;
        }
    }

    public bool SaveActiveFile()
    {
        return _activeFilePath != null && SaveFile(_activeFilePath);
    }

    public void CloseFile(string vfsPath)
    {
        if (_openFiles.Remove(vfsPath))
        {
            if (_activeFilePath == vfsPath)
                _activeFilePath = _openFiles.Keys.FirstOrDefault();

            FileClosed?.Invoke(vfsPath);
        }
    }

    public void CloseAll()
    {
        var paths = _openFiles.Keys.ToList();
        _openFiles.Clear();
        _activeFilePath = null;

        foreach (var path in paths)
            FileClosed?.Invoke(path);
    }

    public bool IsFileOpen(string vfsPath) => _openFiles.ContainsKey(vfsPath);

    public bool IsFileDirty(string vfsPath)
        => _openFiles.TryGetValue(vfsPath, out var entry) && entry.IsDirty;

    /// <summary>获取已打开文件条目（供 View 注册回调）。</summary>
    internal OpenFileEntry? GetEntry(string vfsPath)
        => _openFiles.TryGetValue(vfsPath, out var entry) ? entry : null;

    /// <summary>注册 View 到对应条目。</summary>
    public void RegisterView(string vfsPath, Views.CodeEditorView view)
    {
        if (_openFiles.TryGetValue(vfsPath, out var entry))
            entry.View = view;
    }

    /// <summary>标记文件为脏。</summary>
    public void MarkDirty(string vfsPath)
    {
        if (_openFiles.TryGetValue(vfsPath, out var entry) && !entry.IsDirty)
        {
            entry.IsDirty = true;
            DirtyStateChanged?.Invoke(vfsPath, true);
        }
    }

    /// <summary>获取所有已打开文件路径。</summary>
    internal IReadOnlyCollection<string> OpenFilePaths => _openFiles.Keys;
}
