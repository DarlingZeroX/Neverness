using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// 场景浏览器上下文菜单贡献者——注册内置菜单项。
/// 外部插件可通过 <see cref="EditorMenuRegistry.RegisterContextMenuContributor"/> 扩展。
/// </summary>
public sealed class SceneBrowserContextMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
    {
        // ── 背景菜单（空白区域右键）──
        RegisterBackgroundMenu(ctx);

        // ── 实体菜单（实体节点右键）──
        RegisterEntityMenu(ctx);
    }

    /// <summary>注册背景菜单项：Add Entity 子菜单。</summary>
    private static void RegisterBackgroundMenu(ContextMenuManager ctx)
    {
        var id = SceneBrowserContextMenu.BackgroundId;

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Camera",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_camera",
                DisplayName = "Camera",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                        EntityFactory.CreateCamera(world);
                },
            },
            SortOrder: 100));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Sprite",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_sprite",
                DisplayName = "Sprite",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                        EntityFactory.CreateSprite(world);
                },
            },
            SortOrder: 200));
    }

    /// <summary>注册实体菜单项：Add Entity / Rename / Duplicate / Delete。</summary>
    private static void RegisterEntityMenu(ContextMenuManager ctx)
    {
        var id = SceneBrowserContextMenu.EntityId;

        // ── Add Entity 子菜单 ──
        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Camera",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_camera",
                DisplayName = "Camera",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                        EntityFactory.CreateCamera(world);
                },
            },
            SortOrder: 100));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Sprite",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_sprite",
                DisplayName = "Sprite",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                        EntityFactory.CreateSprite(world);
                },
            },
            SortOrder: 200));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "sep",
            IsSeparator: true,
            SortOrder: 250));

        // ── 实体操作 ──
        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Rename",
            Command: new EditorCommand
            {
                Id = "scene_browser.rename_entity",
                DisplayName = "Rename",
                Execute = _ =>
                {
                    // TODO: 打开重命名输入框
                },
            },
            Shortcut: "F2",
            SortOrder: 300));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Duplicate",
            Command: new EditorCommand
            {
                Id = "scene_browser.duplicate_entity",
                DisplayName = "Duplicate",
                Execute = _ =>
                {
                    // TODO: 复制实体
                },
            },
            Shortcut: "Ctrl+D",
            SortOrder: 400));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Delete",
            Command: new EditorCommand
            {
                Id = "scene_browser.delete_entity",
                DisplayName = "Delete",
                Execute = _ =>
                {
                    var handleObj = ctx.GetContext(SceneBrowserContextMenu.KeyEntityHandle);
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null && handleObj is int handle && handle > 0)
                    {
                        // 通过 Controller 删除实体，确保 SceneBrowser 刷新
                        var controller = EditorCompositionRoot.SceneBrowserController;
                        if (controller != null)
                        {
                            var entity = ((IScene)world).GetEntity(handle);
                            if (entity != null)
                                controller.DeleteEntity(entity);
                        }
                    }
                },
            },
            Shortcut: "Delete",
            SortOrder: 500));
    }
}
