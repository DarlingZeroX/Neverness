using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 菜单注册表——管理菜单项、命令、动态供应器、贡献者。
/// 单例模式，NativeAOT 友好。
/// </summary>
public sealed class MenuRegistryImp
{
    /// <summary>全局单例。</summary>
    public static MenuRegistryImp Instance { get; } = new();

    private readonly List<EditorMenuItem> _items = [];
    private readonly Dictionary<string, EditorCommand> _commands = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, Action<DynamicMenuBuilder>> _dynamicProviders = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<IMenuContributor> _contributors = [];
    private MenuTree? _cachedTree;
    private bool _dirty = true;

    private MenuRegistryImp() { }

    /// <summary>获取或重建菜单树（内部使用，供渲染器调用）。</summary>
    internal MenuTree GetTree()
    {
        if (_dirty || _cachedTree == null)
        {
            RebuildContributors();
            _cachedTree = MenuTreeBuilder.Build(_items);

            // 注入动态菜单
            foreach (var (path, builder) in _dynamicProviders)
            {
                MenuTreeBuilder.AttachDynamic(_cachedTree, path, builder);
            }

            _dirty = false;
        }
        return _cachedTree;
    }

    /// <summary>注册单个菜单项。</summary>
    public void Register(EditorMenuItem item)
    {
        _items.Add(item);
        _dirty = true;
    }

    /// <summary>批量注册菜单项。</summary>
    public void RegisterAll(ReadOnlySpan<EditorMenuItem> items)
    {
        foreach (ref readonly var item in items)
        {
            _items.Add(item);
        }
        _dirty = true;
    }

    /// <summary>注册动态菜单项供应器。</summary>
    public void RegisterDynamic(string menuPath, Action<DynamicMenuBuilder> builder)
    {
        _dynamicProviders[menuPath] = builder;
        _dirty = true;
    }

    /// <summary>注册命令到全局命令表。</summary>
    public void RegisterCommand(EditorCommand command)
    {
        _commands[command.Id] = command;
    }

    /// <summary>执行命令（按 Id 查找并执行）。</summary>
    public bool ExecuteCommand(string commandId)
    {
        if (!_commands.TryGetValue(commandId, out var cmd))
            return false;
        if (cmd.CanExecute != null && !cmd.CanExecute())
            return false;
        cmd.Execute(default);
        return true;
    }

    /// <summary>查找命令（按 Id）。</summary>
    public EditorCommand? FindCommand(string commandId) =>
        _commands.TryGetValue(commandId, out var cmd) ? cmd : null;

    /// <summary>注册菜单贡献者（插件入口）。</summary>
    public void RegisterContributor(IMenuContributor contributor)
    {
        _contributors.Add(contributor);
        _dirty = true;
    }

    /// <summary>移除以指定前缀路径注册的所有菜单项。</summary>
    public void UnregisterByPrefix(string pathPrefix)
    {
        _items.RemoveAll(item => item.Path.StartsWith(pathPrefix, StringComparison.OrdinalIgnoreCase));
        _dirty = true;
    }

    /// <summary>重建所有贡献者注册的菜单项。</summary>
    private void RebuildContributors()
    {
        foreach (var c in _contributors)
        {
            // 贡献者通过 MenuRegistryImp 实例注册菜单项
            c.Build(this);
        }
    }
}
