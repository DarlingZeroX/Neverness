using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Rmlui;

/// <summary>
/// RmlUI 上下文菜单贡献者——在 SceneBrowser 的 "Add Entity/UI/" 子菜单中注册
/// "RmlUI Document" 实体创建项。
/// </summary>
public sealed class RmlUIContextMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
    {
        var id = SceneBrowserContextMenu.BackgroundId;

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/UI/RmlUI Document",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_rmlui_document",
                DisplayName = "RmlUI Document",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                    {
                        EntityFactory.CreateRmlUIDocument(world);
                    }
                },
            },
            Icon: FontAwesome5Pro.FileCode,
            SortOrder: 500));
    }
}
