namespace Neverness.Editor.CodeEditor.Public;

/// <summary>
/// 代码编辑器服务接口——管理代码文件的打开、保存、关闭。
/// </summary>
public interface ICodeEditorService
{
    /// <summary>在编辑器中打开指定 VFS 路径的文件。</summary>
    /// <param name="vfsPath">VFS 虚拟路径（如 /assets/ui/main.html）。</param>
    /// <param name="fileName">显示文件名。</param>
    /// <returns>是否成功打开。</returns>
    bool OpenFile(string vfsPath, string fileName);

    /// <summary>保存指定 VFS 路径的文件（如果已打开且有改动）。</summary>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    /// <returns>是否成功保存。</returns>
    bool SaveFile(string vfsPath);

    /// <summary>保存当前活动的编辑器文件。</summary>
    /// <returns>是否成功保存。</returns>
    bool SaveActiveFile();

    /// <summary>关闭指定 VFS 路径的编辑器标签。</summary>
    /// <param name="vfsPath">VFS 虚拟路径。</param>
    void CloseFile(string vfsPath);

    /// <summary>关闭所有打开的代码编辑器标签。</summary>
    void CloseAll();

    /// <summary>指定路径的文件是否已打开。</summary>
    bool IsFileOpen(string vfsPath);

    /// <summary>指定路径的文件是否有未保存的改动。</summary>
    bool IsFileDirty(string vfsPath);
}
