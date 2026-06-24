using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.ImGuiFrontend.ContextMenus;

/// <summary>
/// 资产创建菜单贡献者（ImGui 版本）——使用 Controller 执行操作。
/// 替代 Assets 模块的 AssetCreationMenuContributor。
/// </summary>
public sealed class AssetCreationImGuiMenuContributor : IContextMenuContributor
{
    private readonly ContentBrowserController _controller;

    public AssetCreationImGuiMenuContributor(ContentBrowserController controller)
    {
        _controller = controller;
    }

    public void Build(ContextMenuManager ctx)
    {
        RegisterCreateMenu(ctx);
    }

    /// <summary>注册 "Create >" 子菜单（按 Category 分组）。</summary>
    private void RegisterCreateMenu(ContextMenuManager ctx)
    {
        var id = "content_browser.background";
        var registry = AssetFactoryRegistry.Instance;

        // 获取所有已注册工厂，按 Category 分组后动态注册
        var factories = registry.Factories;

        // 先按 Category 排序，再按 DisplayName 排序
        var grouped = factories
            .GroupBy(f => f.Category)
            .OrderBy(g => g.Key, StringComparer.OrdinalIgnoreCase);

        int sortOrder = 550; // 插在 separator（500）之后

        foreach (var group in grouped)
        {
            foreach (var factory in group.OrderBy(f => f.DisplayName, StringComparer.OrdinalIgnoreCase))
            {
                // 捕获循环变量
                var capturedFactory = factory;

                ctx.RegisterItem(id, new EditorMenuItem(
                    Path: $"Create {capturedFactory.DisplayName}",
                    Command: new EditorCommand
                    {
                        Id = $"assets.create_{capturedFactory.DisplayName.ToLowerInvariant().Replace(' ', '_')}",
                        DisplayName = $"Create {capturedFactory.DisplayName}",
                        Execute = _ =>
                        {
                            var currentDir = _controller.GetCurrentDirectory();
                            if (!string.IsNullOrEmpty(currentDir))
                            {
                                var createdPath = capturedFactory.CreateAsset(new NPath(currentDir));
                                if (createdPath != null)
                                {
                                    RegisterCreatedAsset(createdPath.Value);
                                    _controller.RefreshDirectory();
                                }
                            }
                        },
                    },
                    Icon: capturedFactory.Icon,
                    SortOrder: sortOrder++));
            }
        }
    }

    /// <summary>注册新创建的资产到 EditorAssetDatabase（生成 .meta、推断 TypeId）。</summary>
    private static void RegisterCreatedAsset(NPath filePath)
    {
        // 1. 获取或创建 .meta（自动分配 GUID）
        var importerName = MetaFileManager.InferImporterName(filePath.Extension);
        var meta = MetaFileManager.GetOrCreateMeta(filePath, importerName);

        // 2. 推断资产类型 ID
        var typeId = AssetMeta.InferAssetTypeId(filePath.Extension);

        // 3. 绝对路径 → VFSService 虚拟路径 → 注册
        var virtualPath = ProjectPaths.GetResourcePath(filePath);
        if (virtualPath is { IsEmpty: false } vp)
        {
            EditorAssetDatabase.Register(vp, meta.Guid, typeId);
        }
    }
}
