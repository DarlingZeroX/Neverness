using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.Inspector;

namespace Neverness.Editor.Media;

/// <summary>
/// 媒体模块入口——注册媒体相关的组件 Inspector 和资产打开器。
///
/// - Inspector 通过 ComponentInspectorRegistry.DiscoverFromAssembly() 注册
/// - AssetOpener 由 AssetOpenerRegistry.Discover() 自动扫描（所有 Neverness.Editor.* 程序集）
/// </summary>
public static class MediaModule
{
    public static void Install()
    {
        // 注册本模块的组件 Inspector（AudioSource / VideoPlayer）
        // Scene 的 ComponentInspectorRegistry 只扫描自身程序集，
        // 外部模块需显式调用 DiscoverFromAssembly
        ComponentInspectorRegistry.DiscoverFromAssembly(typeof(MediaModule).Assembly);

        // 注册场景右键菜单贡献者（Audio Source / Video Player 实体创建）
        EditorMenuRegistry.RegisterContextMenuContributor(new MediaContextMenuContributor());

        Console.WriteLine("[MediaModule] 已注册 Inspector + ContextMenu");
    }
}
