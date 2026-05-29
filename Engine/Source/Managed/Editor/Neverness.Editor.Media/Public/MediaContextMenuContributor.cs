using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Media;

/// <summary>
/// 媒体上下文菜单贡献者——在 SceneBrowser 的 "Add Entity" 菜单中注册
/// "Audio Source" 和 "Video Player" 实体创建项。
/// </summary>
public sealed class MediaContextMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
    {
        var id = SceneBrowserContextMenu.BackgroundId;

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Audio Source",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_audio_source",
                DisplayName = "Audio Source",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                    {
                        EntityFactory.CreateAudioSource(world);
                    }
                },
            },
            SortOrder: 300));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Add Entity/Video Player",
            Command: new EditorCommand
            {
                Id = "scene_browser.create_video_player",
                DisplayName = "Video Player",
                Execute = _ =>
                {
                    var world = ctx.GetContext<SceneWorld>(SceneBrowserContextMenu.KeyActiveWorld);
                    if (world != null)
                    {
                        EntityFactory.CreateVideoPlayer(world);
                    }
                },
            },
            SortOrder: 400));
    }
}
