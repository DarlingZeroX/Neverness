using Avalonia.Controls;
using Avalonia.Threading;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.AvaloniaFrontend.Views;

namespace Neverness.Editor.AvaloniaFrontend.Public;

/// <summary>
/// Avalonia View 工厂——创建 Avalonia 版本的 View。
///
/// 由 AvaloniaFrontend 模块实现，注入到 EditorCompositionRoot。
/// 与 ImGuiViewFactory 对应。
///
/// 设计原则：
/// - View 绑定 ViewModel（数据驱动）
/// - Controller 通过 View 内部访问（View 持有 Controller 引用）
/// - Dock 只做容器，不参与业务数据流
/// - 所有 Avalonia 控件创建必须在 UI 线程执行
/// </summary>
public class AvaloniaViewFactory : IViewFactory
{
    // 创建的 View 引用（供 Dock 布局使用）
    private Control? _consoleView;
    private Control? _sceneBrowserView;
    private Control? _inspectorView;
    private Control? _viewportView;
    private Control? _contentBrowserView;

    /// <summary>Console View。</summary>
    public Control? ConsoleView => _consoleView;

    /// <summary>SceneBrowser View。</summary>
    public Control? SceneBrowserView => _sceneBrowserView;

    /// <summary>Inspector View。</summary>
    public Control? InspectorView => _inspectorView;

    /// <summary>Viewport View。</summary>
    public Control? ViewportView => _viewportView;

    /// <summary>ContentBrowser View。</summary>
    public Control? ContentBrowserView => _contentBrowserView;

    public IEditorPanel CreateConsoleView(ConsolePanelViewModel viewModel)
    {
        return Dispatcher.UIThread.Invoke(() =>
        {
            var view = new ConsolePanelAvaloniaView();
            view.Bind(viewModel);
            _consoleView = view;
            return (IEditorPanel)view;
        });
    }

    public IEditorPanel CreateSceneBrowserView(SceneBrowserViewModel viewModel, SceneBrowserController controller)
    {
        return Dispatcher.UIThread.Invoke(() =>
        {
            var view = new SceneBrowserAvaloniaView();
            view.Bind(viewModel);
            view.SetController(controller);
            _sceneBrowserView = view;
            return (IEditorPanel)view;
        });
    }

    public IEditorPanel CreateInspectorView(InspectorViewModel viewModel, InspectorController controller)
    {
        return Dispatcher.UIThread.Invoke(() =>
        {
            var view = new InspectorAvaloniaView();
            view.Bind(viewModel);
            view.SetController(controller);
            _inspectorView = view;
            return (IEditorPanel)view;
        });
    }

    public IEditorPanel CreateViewportView(EditorViewportViewModel viewModel, EditorViewportController controller)
    {
        return Dispatcher.UIThread.Invoke(() =>
        {
            var view = new ViewportAvaloniaView();
            view.Bind(viewModel);
            view.SetController(controller);
            _viewportView = view;
            return (IEditorPanel)view;
        });
    }

    public IEditorPanel CreateContentBrowserView(ContentBrowserViewModel viewModel, ContentBrowserController controller)
    {
        return Dispatcher.UIThread.Invoke(() =>
        {
            var view = new ContentBrowserAvaloniaView();
            view.Bind(viewModel);
            view.SetController(controller);
            _contentBrowserView = view;
            return (IEditorPanel)view;
        });
    }
}
