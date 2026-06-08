using Hexa.NET.ImGui;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Public.Mvvm;
using System.Numerics;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.ImGuiFrontend.Views;

/// <summary>
/// Detail Inspector ImGui View——读取 InspectorViewModel，渲染 ImGui 组件检查器 UI。
///
/// 从 DetailInspector 迁移，保留原有 UI 风格和交互逻辑。
/// 组件字段绘制通过 IInspectorService.DrawComponentInspector 调用。
/// </summary>
public class DetailInspectorImGuiView : PanelViewBase
{
    private InspectorViewModel? _viewModel;
    private InspectorController? _controller;

    public DetailInspectorImGuiView()
        : base("DetailInspector", FontAwesome5Pro.InfoCircle + " DetailInspector")
    {
    }

    /// <summary>设置 Controller。</summary>
    public void SetController(InspectorController controller)
    {
        _controller = controller;
    }

    public override Type ViewModelType => typeof(InspectorViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (InspectorViewModel)viewModel;
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
    }

    public override void Render()
    {
        if (_viewModel == null) return;

        if (!ImGui.Begin(GetWindowFullName()))
        {
            ImGui.End();
            return;
        }

        if (!_viewModel.HasSelection)
        {
            DrawEmptyState();
            ImGui.End();
            return;
        }

        DrawEntityHeader();
        DrawComponents();
        DrawAddComponentButton();

        ImGui.End();
    }

    // ── 空状态 ──
    private static void DrawEmptyState()
    {
        var cursor = ImGui.GetCursorPos();
        var available = ImGui.GetContentRegionAvail();

        var text = "No entity selected.";
        var textSize = ImGui.CalcTextSize(text);

        ImGui.SetCursorPos(new Vector2(
            cursor.X + (available.X - textSize.X) * 0.5f,
            cursor.Y + available.Y * 0.3f));
        ImGui.TextDisabled(text);

        var subText = "Select an entity in the Scene Browser.";
        var subTextSize = ImGui.CalcTextSize(subText);
        ImGui.SetCursorPos(new Vector2(
            cursor.X + (available.X - subTextSize.X) * 0.5f,
            cursor.Y + available.Y * 0.3f + textSize.Y + 8f));
        ImGui.TextDisabled(subText);
    }

    // ── 实体头部 ──
    private void DrawEntityHeader()
    {
        // 实体名称 + 句柄
        ImGui.Text(_viewModel!.SelectedEntityName);
        ImGui.SameLine();
        ImGui.TextDisabled($"  (0x{_viewModel.SelectedEntityHandle:X16})");

        // Active 状态
        ImGui.Spacing();

        bool active = _viewModel.IsActive;
        if (ImGui.Checkbox("Active", ref active))
        {
            _controller?.SetActive(active);
        }

        ImGui.Separator();
    }

    // ── 组件列表 ──
    private void DrawComponents()
    {
        if (_controller == null) return;

        foreach (var component in _viewModel!.Components)
        {
            // PushID 避免 ImGui ID 冲突
            ImGui.PushID((int)(component.TypeId & 0x7FFFFFFF));

            // 折叠面板头（Unity 风格）
            bool open = ImGui.CollapsingHeader(component.DisplayName, ImGuiTreeNodeFlags.DefaultOpen);

            // 右键菜单：移除组件
            if (ImGui.BeginPopupContextItem("##comp_ctx"))
            {
                if (ImGui.MenuItem("Remove Component"))
                {
                    _controller.RemoveComponent(component.TypeId);
                    ImGui.EndPopup();
                    ImGui.PopID();
                    return;
                }
                ImGui.EndPopup();
            }

            // 展开时绘制字段
            if (open)
            {
                ImGui.Indent(12f);
                bool modified = _controller.DrawComponentInspector(component.TypeId);
                ImGui.Unindent(12f);

                // 修改后可在此触发额外逻辑（如标记场景 dirty）
                if (modified)
                {
                    // TODO: 通知场景有未保存修改
                }
            }

            ImGui.PopID();
        }
    }

    // ── 添加组件 ──
    private void DrawAddComponentButton()
    {
        if (_controller == null) return;

        ImGui.Spacing();

        // 居中按钮
        float availWidth = ImGui.GetContentRegionAvail().X;
        float buttonWidth = 200f;
        ImGui.SetCursorPosX(ImGui.GetCursorPosX() + (availWidth - buttonWidth) * 0.5f);

        if (ImGui.Button(FontAwesome5Pro.Plus + " Add Component", new Vector2(buttonWidth, 0)))
        {
            ImGui.OpenPopup("##add_component_popup");
        }

        DrawAddComponentPopup();
    }

    private void DrawAddComponentPopup()
    {
        if (_controller == null) return;

        if (!ImGui.BeginPopup("##add_component_popup"))
            return;

        // 获取可用组件类型
        var availableTypes = _controller.GetAvailableComponentTypes();
        var currentComponents = _viewModel!.Components;

        bool hasAny = false;

        foreach (var type in availableTypes)
        {
            // 跳过已有组件
            if (currentComponents.Any(c => c.TypeId == type.TypeId))
                continue;

            hasAny = true;
            if (ImGui.MenuItem(type.DisplayName))
            {
                _controller.AddComponent(type.TypeId);
            }
        }

        if (!hasAny)
        {
            ImGui.TextDisabled("No more components available.");
        }

        ImGui.EndPopup();
    }

    private void OnPropertyChanged(string propertyName)
    {
        // ImGui 即时模式
    }
}
