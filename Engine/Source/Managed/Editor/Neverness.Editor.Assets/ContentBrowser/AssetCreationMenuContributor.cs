using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Assets.Private.Context;
using Neverness.Editor.Assets.Private.Core;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产创建菜单贡献者——根据 <see cref="AssetFactoryRegistry"/> 中已注册的工厂，
/// 动态生成 ContentBrowser 右键菜单的 "Create >" 子菜单。
/// 空白区域右键：列出所有工厂。
/// 项目右键：继承原有 Rename / Remove 等菜单。
/// </summary>
public sealed class AssetCreationMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
    {
        RegisterCreateMenu(ctx);
    }

    /// <summary>注册 "Create >" 子菜单（按 Category 分组）。</summary>
    private static void RegisterCreateMenu(ContextMenuManager ctx)
    {
        var id = ContentBrowserContextMenu.BackgroundId;
        var registry = AssetFactoryRegistry.Instance;

        // 获取所有已注册工厂，按 Category 分组后动态注册
        var factories = registry.Factories;

        // 先按 Category 排序，再按 DisplayName 排序
        var grouped = factories
            .GroupBy(f => f.Category)
            .OrderBy(g => g.Key, StringComparer.OrdinalIgnoreCase);

        int sortOrder = 550; // 插在 separator（500）之后、原有 Create Scene（600）之前

        foreach (var group in grouped)
        {
            // 分类标题作为分隔提示（不可点击）
            // 实际菜单项直接列出，不使用子菜单层级（ImGui 原生子菜单较复杂）

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
                            var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                            var path = ContextMenuManager.Instance.GetContext<string>(ContentBrowserContextMenu.KeyPath);
                            if (cb != null && path != null)
                            {
                                var createdPath = capturedFactory.CreateAsset(new NPath(path));
                                if (createdPath != null)
                                {
                                    RegisterCreatedAsset(createdPath.Value);
                                    cb.RefreshDirectory();
                                    cb.RefreshDirectoryTreeRoot();
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

        // 3. 绝对路径 → VFS 虚拟路径 → 注册
        var virtualPath = ProjectPaths.GetResourcePath(filePath);
        if (virtualPath is { IsEmpty: false } vp)
        {
            EditorAssetDatabase.Register(vp, meta.Guid, typeId);
        }
    }
}
