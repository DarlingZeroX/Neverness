using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// 场景浏览器上下文菜单贡献者——注册内置的 "Add Entity" 菜单项。
/// 外部插件可通过 <see cref="EditorMenuRegistry.RegisterContextMenuContributor"/> 扩展。
/// </summary>
public sealed class SceneBrowserContextMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
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
}
