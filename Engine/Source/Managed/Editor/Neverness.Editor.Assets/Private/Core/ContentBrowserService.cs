using Neverness.Editor.Core.Public;

namespace Neverness.Editor.Assets.Private.Core;

/// <summary>
/// <see cref="IContentBrowserService"/> 的具体实现——包装 <see cref="ContentBrowser"/> 单例，
/// 通过 Core 服务定位器暴露给 Scene / Inspector 等外部模块。
/// </summary>
internal sealed class ContentBrowserService : IContentBrowserService
{
    private ContentBrowser Browser =>
        ContentBrowser.Instance
        ?? throw new InvalidOperationException("ContentBrowser is not initialized.");

    public string CurrentDirectory => Browser.GetCurrentBrowserDirectory();

    public string ProjectDirectory => Browser.GetAssetDirectory();

    public void Refresh() => Browser.RefreshDirectory();

    public void OpenDirectory(string absolutePath) => Browser.OpenDirectory(absolutePath);

    public IReadOnlyList<string> GetSubdirectories()
    {
        var node = Browser.GetCurrentDirectoryNode();
        return node.Directories.Select(d => d.Name).ToList();
    }

    public IReadOnlyList<string> GetFiles()
    {
        var node = Browser.GetCurrentDirectoryNode();
        return node.Files.Select(f => f.Name + f.Extension).ToList();
    }
}
