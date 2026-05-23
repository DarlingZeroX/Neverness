using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 菜单贡献者接口——插件通过此接口扩展菜单。
/// </summary>
public interface IMenuContributor
{
    /// <summary>向注册表贡献菜单项和命令。</summary>
    void Build(MenuRegistryImp registry);
}
