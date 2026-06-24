using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Threading;
using Neverness.Editor.Assets.Private.Context;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;
using AssetsContentBrowser = Neverness.Editor.Assets.Private.Core.ContentBrowser;
using AssetsContentFile = Neverness.Editor.Assets.Private.Core.ContentFile;
using AssetsContentDirectory = Neverness.Editor.Assets.Private.Core.ContentDirectory;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器交互逻辑——右键菜单、ViewModel 变更处理。
/// </summary>
internal sealed class ContentBrowserInteractions
{
    private readonly ContentBrowserViewModel _viewModel;
    private readonly ContentBrowserController _controller;
    private readonly ContentBrowserToolbar _toolbar;
    private readonly ContentBrowserStatusBar _statusBar;
    private readonly ContentBrowserDirectoryTree _directoryTree;
    private readonly ContentBrowserThumbnailGrid _thumbnailGrid;
    private readonly AvaloniaContextMenuRenderer _contextMenuRenderer;

    internal ContentBrowserInteractions(
        ContentBrowserViewModel viewModel,
        ContentBrowserController controller,
        ContentBrowserToolbar toolbar,
        ContentBrowserStatusBar statusBar,
        ContentBrowserDirectoryTree directoryTree,
        ContentBrowserThumbnailGrid thumbnailGrid)
    {
        _viewModel = viewModel;
        _controller = controller;
        _toolbar = toolbar;
        _statusBar = statusBar;
        _directoryTree = directoryTree;
        _thumbnailGrid = thumbnailGrid;
        _contextMenuRenderer = new AvaloniaContextMenuRenderer();

        // 订阅事件
        _viewModel.PropertyChanged += OnPropertyChanged;
        _thumbnailGrid.OnContextMenuRequested += OnThumbnailContextMenu;
        _thumbnailGrid.OnBackgroundContextMenuRequested += OnBackgroundContextMenu;
        _thumbnailGrid.OnRenameCommitted += OnRenameCommitted;
        _directoryTree.DirectoryTree!.PointerReleased += OnDirectoryTreePointerReleased;

        // 注册重命名回调到上下文（供 ContextMenuContributor 调用）
        var ctx = ContextMenuManager.Instance;
        ctx.SetContext("content_browser.begin_rename", (Action<string, string>)((path, name) =>
            Dispatcher.UIThread.Post(() => _thumbnailGrid.BeginRename(path, name))));
    }

    /// <summary>释放资源。</summary>
    internal void Dispose()
    {
        _viewModel.PropertyChanged -= OnPropertyChanged;
        _thumbnailGrid.OnContextMenuRequested -= OnThumbnailContextMenu;
        _thumbnailGrid.OnBackgroundContextMenuRequested -= OnBackgroundContextMenu;
        _thumbnailGrid.OnRenameCommitted -= OnRenameCommitted;
        if (_directoryTree.DirectoryTree != null)
            _directoryTree.DirectoryTree.PointerReleased -= OnDirectoryTreePointerReleased;
    }

    /* ======================== ViewModel 变更 ======================== */

    private void OnPropertyChanged(string? propertyName)
    {
        // PropertyChanged 可能在后台线程触发（如 Task.Run 中的 RefreshDirectory），
        // 必须切到 UI 线程执行 UI 操作
        Dispatcher.UIThread.Invoke(() =>
        {
            if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
            {
                _thumbnailGrid.ClearSelection();
                _toolbar.RefreshBreadcrumb();
                _thumbnailGrid.Refresh();
                _directoryTree.HighlightNode(_viewModel.CurrentDirectory);
                _statusBar.Update(_thumbnailGrid.SelectedPaths.Count);
            }
            else if (propertyName == nameof(ContentBrowserViewModel.SearchFilter))
            {
                _thumbnailGrid.Refresh();
                _statusBar.Update(_thumbnailGrid.SelectedPaths.Count);
            }
        });
    }

    /* ======================== 右键菜单 ======================== */

    private void OnBackgroundContextMenu(Control target)
    {
        var ctx = ContextMenuManager.Instance;
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, _viewModel.CurrentDirectory ?? "");
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

        _contextMenuRenderer.BuildAndShow(ContentBrowserContextMenu.BackgroundId, ctx, target);
    }

    private void OnThumbnailContextMenu(Control target, string path, string name, bool isDirectory)
    {
        var ctx = ContextMenuManager.Instance;
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

        Neverness.Editor.Assets.Private.Core.ContentItem contentItem = isDirectory
            ? new AssetsContentDirectory { Name = name, SystemPath = new NPath(path) }
            : new AssetsContentFile { Name = name, SystemPath = new NPath(path), Extension = System.IO.Path.GetExtension(path) };
        ctx.SetContext(ContentBrowserContextMenu.KeyItem, contentItem);

        _contextMenuRenderer.BuildAndShow(ContentBrowserContextMenu.ItemId, ctx, target);
    }

    private void OnDirectoryTreePointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;

        if (_directoryTree.DirectoryTree?.SelectedItem is TreeViewItem item && item.Tag is string path)
        {
            var name = (item.Header as StackPanel)?.Children
                .OfType<TextBlock>().LastOrDefault()?.Text ?? "";

            _thumbnailGrid.SetSelectedItem(path, name, true);

            var ctx = ContextMenuManager.Instance;
            ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
            ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

            _contextMenuRenderer.BuildAndShow(ContentBrowserContextMenu.BackgroundId, ctx, item);
        }

        e.Handled = true;
    }

    /* ======================== 重命名 ======================== */

    private void OnRenameCommitted(string path, string newName)
    {
        _controller.RenameItem(path, newName);
    }
}
