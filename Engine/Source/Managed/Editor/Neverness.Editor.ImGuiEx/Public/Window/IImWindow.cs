using Hexa.NET.ImGui;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// ImGui 窗口接口，定义窗口的基础属性和生命周期。
/// </summary>
public interface IImWindow
{
    /// <summary>窗口唯一标识（构造时自动生成 Guid）。</summary>
    Guid WindowId { get; }

    /// <summary>窗口显示标题。</summary>
    string Title { get; set; }

    /// <summary>窗口是否处于打开状态（用户可点击关闭按钮关闭）。</summary>
    bool IsOpen { get; }

    /// <summary>窗口当前是否获得焦点。</summary>
    bool IsFocused { get; }

    /// <summary>窗口是否可见（控制是否参与渲染）。</summary>
    bool IsVisible { get; set; }

    /// <summary>ImGui 窗口标志位。</summary>
    ImGuiWindowFlags Flags { get; set; }

    /// <summary>窗口行为标志位。</summary>
    ImWindowBehaviorFlags BehaviorFlags { get; set; }

    /// <summary>停靠目标 DockNode ID（0 表示不指定）。</summary>
    uint DockId { get; set; }

    /// <summary>窗口首次打开时调用。</summary>
    void OnOpen();

    /// <summary>窗口关闭时调用。</summary>
    void OnClose();

    /// <summary>窗口获得焦点时调用。</summary>
    void OnFocus();

    /// <summary>窗口失去焦点时调用。</summary>
    void OnLostFocus();

    /// <summary>每帧更新逻辑（非 GUI）。</summary>
    void OnUpdate(float deltaTime);

    /// <summary>每帧渲染（调用 ImGui.Begin/End + 内容绘制）。</summary>
    void Render();

    /// <summary>请求关闭窗口。</summary>
    void RequestClose();

    /// <summary>
    /// 返回 ImGui 窗口唯一 ID 字符串，格式 "Title##GUID_HEX"。
    /// 保证同类型多实例窗口在 ImGui 中不合并。
    /// </summary>
    string GetImGuiId();
}
