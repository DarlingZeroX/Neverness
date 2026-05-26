namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 窗口行为标志位，用于控制 ImWindow 的实例化、停靠、持久化等行为。
/// </summary>
[Flags]
public enum ImWindowBehaviorFlags
{
    None = 0,

    /// <summary>允许同一类型同时打开多个实例。</summary>
    MultiInstance = 1 << 0,

    /// <summary>窗口布局参与 ImGui INI 持久化。</summary>
    SaveLayout = 1 << 1,

    /// <summary>窗口自带菜单栏。</summary>
    HasMenuBar = 1 << 2,

    /// <summary>窗口允许被停靠到 DockSpace。</summary>
    AllowDocking = 1 << 3,

    /// <summary>默认行为：允许停靠 + 布局持久化。</summary>
    Default = SaveLayout | AllowDocking,
}
