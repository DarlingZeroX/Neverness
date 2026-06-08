namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 工具栏注册表——存储工具栏按钮描述符（UI 无关）。
/// Framework 的 ModuleImp 通过此类注册工具栏按钮。
/// ImGuiFrontend 的 IToolbarRenderer 从此类读取按钮列表进行渲染。
/// </summary>
public sealed class ToolbarRegistry
{
    /// <summary>全局单例。</summary>
    public static ToolbarRegistry Instance { get; } = new();

    private readonly List<ToolbarCommand> _commands = [];

    private ToolbarRegistry() { }

    /// <summary>注册工具栏按钮。</summary>
    public void Register(ToolbarCommand cmd) => _commands.Add(cmd);

    /// <summary>获取所有已注册的工具栏按钮（按 SortOrder 排序）。</summary>
    public IReadOnlyList<ToolbarCommand> GetCommands() =>
        _commands.OrderBy(c => c.SortOrder).ToList();
}
