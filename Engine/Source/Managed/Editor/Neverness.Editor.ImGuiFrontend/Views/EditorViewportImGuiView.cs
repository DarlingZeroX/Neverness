using Hexa.NET.ImGui;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Public.Mvvm;
using System.Numerics;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.ImGuiFrontend.Views;

/// <summary>
/// 编辑器视口 ImGui View——读取 EditorViewportViewModel，渲染 ImGui 视口。
///
/// Phase 6 迁移对象：场景渲染纹理显示、RmlUI 叠加层。
/// </summary>
public class EditorViewportImGuiView : PanelViewBase
{
    private EditorViewportViewModel? _viewModel;
    private EditorViewportController? _controller;

    public EditorViewportImGuiView()
        : base("Main Viewport", "🖥️ Main Viewport")
    {
    }

    /// <summary>设置 Controller。</summary>
    public void SetController(EditorViewportController controller)
    {
        _controller = controller;
    }

    public override Type ViewModelType => typeof(EditorViewportViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (EditorViewportViewModel)viewModel;
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
    }

    public override unsafe void Render()
    {
        if (_viewModel == null) return;

        ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, Vector2.Zero);

        if (!ImGui.Begin(GetWindowFullName(),
            ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.NoCollapse))
        {
            ImGui.End();
            ImGui.PopStyleVar();
            return;
        }

        var viewportSize = ImGui.GetContentRegionAvail();

        if (viewportSize.X > 1 && viewportSize.Y > 1 && _viewModel.HasScene)
        {
            // 更新视口尺寸
            _controller?.UpdateViewportSize(viewportSize.X, viewportSize.Y);

            // 已移除：RenderScene()（Legacy Scene 渲染路径已删除）
            // ImGuiFrontend 仅保留作历史参考，使用 RenderViewportCommands 路径替代
        }

        ImGui.End();
        ImGui.PopStyleVar();
    }

    private void OnPropertyChanged(string propertyName)
    {
        // ImGui 即时模式
    }
}
