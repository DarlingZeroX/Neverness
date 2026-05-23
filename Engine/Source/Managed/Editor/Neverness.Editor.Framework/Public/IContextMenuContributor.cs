using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 上下文菜单贡献者接口——插件通过此接口扩展上下文菜单。
/// 与 <see cref="IMenuContributor"/> 对称，用于右键菜单的可泛化扩展。
/// </summary>
public interface IContextMenuContributor
{
    /// <summary>
    /// 向上下文菜单管理器注册菜单项。
    /// 使用 <see cref="ContextMenuManager.RegisterItem"/> 注册到指定 contextId。
    /// </summary>
    void Build(ContextMenuManager ctx);
}
