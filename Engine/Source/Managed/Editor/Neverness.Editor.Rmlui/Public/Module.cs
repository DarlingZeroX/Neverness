using Neverness.Editor.Framework.Public;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Public.Inspector;

namespace Neverness.Editor.Rmlui.Public;

/// <summary>
/// Neverness.Editor.Rmlui 模块安装入口。
///
/// - RmlDocumentAssetFactory 通过 AssetFactoryRegistry 自动发现
/// - Inspector 通过 ComponentInspectorRegistry.DiscoverFromAssembly() 注册
/// - ContextMenuContributor 通过 EditorMenuRegistry.RegisterContextMenuContributor() 注册
/// - RmlDocumentReloadService 订阅 AssetReloaded 事件，通知 native 端热重载
/// </summary>
public static class RmluiModule
{
    private static RmlDocumentReloadService? s_reloadService;

    /// <summary>安装 RmlUI 模块。</summary>
    public static void Install()
    {
        // 注册本模块的组件 Inspector（RmlUIDocument）
        // Scene 的 ComponentInspectorRegistry 只扫描自身程序集，
        // 外部模块需显式调用 DiscoverFromAssembly
        ComponentInspectorRegistry.DiscoverFromAssembly(typeof(RmluiModule).Assembly);

        // 注册场景右键菜单贡献者（UI/RmlUI Document 实体创建）
        EditorMenuRegistry.RegisterContextMenuContributor(new RmlUIContextMenuContributor());

        // 注册 RmlUI 文档热重载服务（订阅 AssetReloaded 事件）
        s_reloadService = new RmlDocumentReloadService(EditorCoreModule.Context.Events);

        Console.WriteLine("[RmluiModule] 已注册 Inspector + ContextMenu + HotReload");
    }
}
