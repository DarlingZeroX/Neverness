using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.ImGuiFrontend.Views;

namespace Neverness.Editor.ImGuiFrontend.Public;

/// <summary>
/// ImGui View 工厂——创建 ImGui 版本的 View。
///
/// 由 ImGuiFrontend 模块实现，注入到 EditorCompositionRoot。
/// 未来 AvaloniaFrontend 只需提供 AvaloniaViewFactory 即可。
/// </summary>
public class ImGuiViewFactory : IViewFactory
{
    public IEditorPanel CreateConsoleView(ConsolePanelViewModel viewModel)
    {
        var view = new ConsolePanelImGuiView();
        view.Bind(viewModel);
        return view;
    }

    public IEditorPanel CreateSceneBrowserView(SceneBrowserViewModel viewModel, SceneBrowserController controller)
    {
        var view = new SceneBrowserImGuiView();
        view.Bind(viewModel);
        // Controller 通过 View 内部访问（View 持有 Controller 引用）
        view.SetController(controller);
        return view;
    }

    public IEditorPanel CreateInspectorView(InspectorViewModel viewModel, InspectorController controller)
    {
        var view = new DetailInspectorImGuiView();
        view.Bind(viewModel);
        view.SetController(controller);
        return view;
    }

    public IEditorPanel CreateViewportView(EditorViewportViewModel viewModel, EditorViewportController controller)
    {
        var view = new EditorViewportImGuiView();
        view.Bind(viewModel);
        view.SetController(controller);
        return view;
    }

    public IEditorPanel CreateContentBrowserView(ContentBrowserViewModel viewModel, ContentBrowserController controller)
    {
        var view = new ContentBrowserImGuiView();
        view.Bind(viewModel);
        view.SetController(controller);
        return view;
    }
}
