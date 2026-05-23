using Neverness.Editor.Assets.AssetActions;
using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Assets.Private;

/// <summary>
/// 资产工厂模块内部安装实现。
/// </summary>
internal static class AssetsModuleImp
{
    public static void Install()
    {
        // 1. 触发工厂自动发现（扫描所有 Neverness.Editor.* 程序集）
        var factories = AssetFactoryRegistry.Instance.Factories;

        // 2. 注册资产类型操作元数据
        var actions = AssetTypeActionsRegistry.Instance;
        foreach (var factory in factories)
        {
            actions.Register(new AssetTypeActions
            {
                DisplayName = factory.DisplayName,
                FileExtension = factory.FileExtension,
                Icon = factory.Icon,
                Category = factory.Category,
                CanOpen = true,
                CanImport = false,
            });
        }

        // 3. 注册 ContentBrowser 上下文菜单贡献者（动态 Create 子菜单）
        EditorMenuRegistry.RegisterContextMenuContributor(new AssetCreationMenuContributor());
    }
}
