namespace Neverness.Editor.Core.Public;

/// <summary>
/// ContentBrowser 服务接口——通过 Core 服务定位器暴露。
/// Scene / Inspector 等模块可消费此接口，无需直接引用 Assets 程序集。
/// </summary>
public interface IContentBrowserService
{
    /// <summary>当前浏览目录的绝对路径。</summary>
    string CurrentDirectory { get; }

    /// <summary>项目资产根目录的绝对路径。</summary>
    string ProjectDirectory { get; }

    /// <summary>刷新当前目录。</summary>
    void Refresh();

    /// <summary>打开指定目录。</summary>
    void OpenDirectory(string absolutePath);

    /// <summary>当前目录下的所有子目录名。</summary>
    IReadOnlyList<string> GetSubdirectories();

    /// <summary>当前目录下的所有文件名（含扩展名）。</summary>
    IReadOnlyList<string> GetFiles();
}
