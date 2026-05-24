using Neverness.Editor.Core.Private.Panel;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// Editor.Core 模块实现——创建编辑器上下文、初始化生命周期管理器。
/// </summary>
public static class CoreModuleImp
{
    private static EditorContext? _context;
    private static ModuleLifecycleManager? _lifecycleManager;

    /// <summary>编辑器上下文（Install 后可用）。</summary>
    public static IEditorContext Context =>
        _context ?? throw new InvalidOperationException("EditorCoreModule is not installed.");

    /// <summary>生命周期管理器（Install 后可用）。</summary>
    public static ModuleLifecycleManager LifecycleManager =>
        _lifecycleManager ?? throw new InvalidOperationException("EditorCoreModule is not installed.");

    public static void Install()
    {
        // 注册内置菜单贡献者（File / Edit / Window / Help）
        EditorMenuRegistry.RegisterContributor(new BuiltinMenuContributor());

        // 添加 Console 面板到主窗口
        PanelManager.Instance.AddChildPanel("Console", new ConsolePanel());

        // 创建编辑器上下文
        _context = new EditorContext();

        // 创建生命周期管理器
        _lifecycleManager = new ModuleLifecycleManager();

        // 自动发现当前程序集中的 Feature
        _lifecycleManager.DiscoverFeatures(typeof(CoreModuleImp).Assembly);

        // 按拓扑排序初始化所有 Feature
        _lifecycleManager.InitializeAll(_context);
    }

    public static void Shutdown()
    {
        if (_lifecycleManager != null && _context != null)
        {
            _lifecycleManager.ShutdownAll(_context);
        }

        _lifecycleManager = null;
        _context = null;
    }
}
