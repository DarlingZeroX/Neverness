using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// 编辑器上下文实现——聚合 Framework 提供的基础设施接口。
/// Feature 通过此上下文访问面板、菜单、命令、事件等全局服务。
/// </summary>
public sealed class EditorContext : IEditorContext
{
    private readonly Dictionary<Type, object> _services = new();

    public IPanelManager Panels { get; }
    public ICommandRegistry Commands { get; }
    public IMenuRegistry Menus { get; }
    public IContextMenuRegistry ContextMenus { get; }
    public IEditorEventBus Events { get; }
    public EditorState State { get; }

    public EditorContext()
    {
        Panels = PanelManager.Instance;
        Commands = new CommandRegistry();
        Menus = MenuRegistryImp.Instance;
        ContextMenus = ContextMenuManager.Instance;
        Events = new EditorEventBus();
        State = new EditorState();
    }

    public T GetService<T>() where T : class
    {
        if (_services.TryGetValue(typeof(T), out var service))
        {
            return (T)service;
        }

        throw new InvalidOperationException(
            $"Service of type '{typeof(T).Name}' is not registered.");
    }

    public void RegisterService<T>(T service) where T : class
    {
        ArgumentNullException.ThrowIfNull(service);
        _services[typeof(T)] = service;
    }

    public bool TryGetService<T>(out T service) where T : class
    {
        if (_services.TryGetValue(typeof(T), out var obj) && obj is T typed)
        {
            service = typed;
            return true;
        }

        service = default!;
        return false;
    }
}
