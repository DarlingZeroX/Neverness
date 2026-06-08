using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 菜单渲染器接口——由 ImGuiFrontend 实现。
///
/// 封装 ImGui 主菜单栏渲染，使 Shell 模块无需直接引用 ImGui。
/// </summary>
public interface IMenuRenderer
{
    /// <summary>渲染主菜单栏（完整生命周期：Begin → 菜单项 → End）。</summary>
    void RenderMainMenuBar(MenuTree tree);

    /// <summary>开始主菜单栏渲染。</summary>
    bool BeginMainMenuBar();

    /// <summary>结束主菜单栏渲染。</summary>
    void EndMainMenuBar();

    /// <summary>仅渲染菜单项（不含 Begin/End 包裹）。</summary>
    void RenderMenuItems(MenuTree tree);
}

/// <summary>
/// 工具栏渲染器接口——由 ImGuiFrontend 实现。
/// </summary>
public interface IToolbarRenderer
{
    /// <summary>注册工具栏按钮。</summary>
    void Register(ToolbarCommand cmd);

    /// <summary>渲染工具栏。</summary>
    void Render();
}
