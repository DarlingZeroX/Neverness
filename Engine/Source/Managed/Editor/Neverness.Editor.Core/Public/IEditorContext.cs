using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器运行时上下文——所有 Feature 通过此上下文访问全局服务。
/// 替代当前散落的 singleton 模式。
/// </summary>
public interface IEditorContext
{
    /// <summary>面板管理器（Framework 提供）。</summary>
    IPanelManager Panels { get; }

    /// <summary>命令注册表（Legacy IEditorCommand 体系）。</summary>
    ICommandRegistry Commands { get; }

    /// <summary>菜单注册表。</summary>
    IMenuRegistry Menus { get; }

    /// <summary>上下文菜单管理器。</summary>
    IContextMenuRegistry ContextMenus { get; }

    /// <summary>编辑器事件总线。</summary>
    IEditorEventBus Events { get; }

    /// <summary>编辑器状态。</summary>
    EditorState State { get; }

    /// <summary>获取已注册的服务。</summary>
    T GetService<T>() where T : class;

    /// <summary>注册服务。</summary>
    void RegisterService<T>(T service) where T : class;

    /// <summary>尝试获取已注册的服务。</summary>
    bool TryGetService<T>(out T service) where T : class;
}
