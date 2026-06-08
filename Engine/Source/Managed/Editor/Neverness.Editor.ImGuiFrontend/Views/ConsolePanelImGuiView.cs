using Hexa.NET.ImGui;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.ImGuiFrontend.Views;

/// <summary>
/// 控制台 ImGui View——读取 ConsoleViewModel，渲染 ImGui UI。
///
/// Phase 2 验证对象：最简单的 MVVM 面板，用于验证基础架构是否可行。
/// </summary>
public class ConsolePanelImGuiView : PanelViewBase
{
    private ConsolePanelViewModel? _viewModel;

    public ConsolePanelImGuiView() : base("Console") { }

    public override Type ViewModelType => typeof(ConsolePanelViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ConsolePanelViewModel)viewModel;
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

        RenderToolbar();
        ImGui.Separator();
        RenderLogList();

        ImGui.End();
    }

    // ── 工具栏 ──
    private void RenderToolbar()
    {
        // 清空按钮
        if (ImGui.Button("Clear"))
        {
            _viewModel!.Clear();
        }

        ImGui.SameLine();

        // 自动滚动开关
        var autoScroll = _viewModel!.AutoScroll;
        if (ImGui.Checkbox("Auto Scroll", ref autoScroll))
        {
            _viewModel.AutoScroll = autoScroll;
        }

        ImGui.SameLine();

        // 过滤输入框
        var filterText = _viewModel.FilterText;
        ImGui.SetNextItemWidth(200);
        if (ImGui.InputTextWithHint("##filter", "Filter...", ref filterText, 256))
        {
            _viewModel.FilterText = filterText;
        }

        ImGui.SameLine();

        // 日志级别过滤
        RenderLogLevelFilter();
    }

    private void RenderLogLevelFilter()
    {
        var level = _viewModel!.FilterLevel;
        var levelText = level switch
        {
            LogLevel.All => "All",
            LogLevel.Debug => "Debug",
            LogLevel.Info => "Info",
            LogLevel.Warning => "Warning",
            LogLevel.Error => "Error",
            LogLevel.Fatal => "Fatal",
            _ => "Custom"
        };

        if (ImGui.BeginCombo("##level", levelText))
        {
            if (ImGui.Selectable("All", level == LogLevel.All))
                _viewModel.FilterLevel = LogLevel.All;
            if (ImGui.Selectable("Debug", level == LogLevel.Debug))
                _viewModel.FilterLevel = LogLevel.Debug;
            if (ImGui.Selectable("Info", level == LogLevel.Info))
                _viewModel.FilterLevel = LogLevel.Info;
            if (ImGui.Selectable("Warning", level == LogLevel.Warning))
                _viewModel.FilterLevel = LogLevel.Warning;
            if (ImGui.Selectable("Error", level == LogLevel.Error))
                _viewModel.FilterLevel = LogLevel.Error;
            if (ImGui.Selectable("Fatal", level == LogLevel.Fatal))
                _viewModel.FilterLevel = LogLevel.Fatal;
            ImGui.EndCombo();
        }
    }

    // ── 日志列表 ──
    private void RenderLogList()
    {
        if (ImGui.BeginChild("LogRegion"))
        {
            var entries = _viewModel!.FilteredEntries;
            foreach (var entry in entries)
            {
                // 根据日志级别着色
                var color = entry.Level switch
                {
                    LogLevel.Debug => new System.Numerics.Vector4(0.6f, 0.6f, 0.6f, 1.0f),
                    LogLevel.Info => new System.Numerics.Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    LogLevel.Warning => new System.Numerics.Vector4(1.0f, 1.0f, 0.0f, 1.0f),
                    LogLevel.Error => new System.Numerics.Vector4(1.0f, 0.3f, 0.3f, 1.0f),
                    LogLevel.Fatal => new System.Numerics.Vector4(1.0f, 0.0f, 0.0f, 1.0f),
                    _ => new System.Numerics.Vector4(1.0f, 1.0f, 1.0f, 1.0f)
                };

                ImGui.PushStyleColor(ImGuiCol.Text, color);
                ImGui.TextUnformatted($"[{entry.Level}] {entry.Message}");
                ImGui.PopStyleColor();
            }

            // 自动滚动到底部
            if (_viewModel.AutoScroll && ImGui.GetScrollY() >= ImGui.GetScrollMaxY() - 20)
            {
                ImGui.SetScrollHereY(1.0f);
            }

            ImGui.EndChild();
        }
    }

    private void OnPropertyChanged(string propertyName)
    {
        // ImGui 即时模式，每帧重新读取 ViewModel
    }
}
