using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 上下文菜单渲染器接口——由 ImGuiFrontend 实现。
///
/// ContextMenuManager 负责注册逻辑（UI 无关），
/// 此接口负责 ImGui 渲染逻辑。
/// </summary>
public interface IContextMenuRenderer
{
    /// <summary>开始窗口级弹出菜单（右键空白区域）。</summary>
    bool BeginWindowPopup(string contextId);

    /// <summary>开始项目级弹出菜单（右键某个项目）。</summary>
    bool BeginItemPopup(string contextId);

    /// <summary>结束弹出菜单。</summary>
    void EndPopup();

    /// <summary>在弹出菜单内渲染所有已注册的内容。</summary>
    void RenderPopupContent(string contextId, ContextMenuManager manager);

    /// <summary>完整渲染窗口级上下文菜单（Begin → 内容 → End）。</summary>
    void RenderWindowContextMenu(string contextId, ContextMenuManager manager);

    /// <summary>完整渲染项目级上下文菜单（Begin → 内容 → End）。</summary>
    void RenderItemContextMenu(string contextId, ContextMenuManager manager);
}
