using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.ProjectSystem.Public;

public static class ProjectPaths
{
    public static NVirtualPath Project { get; } = new("/project/");
    public static NVirtualPath Assets { get; } = new("/assets/");
    public static NVirtualPath Library { get; } = new("/Library/");

    public static NVirtualPath Intermediate { get; } = new("/projectIntermediate/");

    public static NVirtualPath Settings { get; } = new("/projectSettings/");
    public static NVirtualPath EngineResource { get; } = new("/engine/");
    public static NVirtualPath EditorResource { get; } = new("/editor/");
    public static NVirtualPath DefaultSpriteTexture { get; } = new("/engine/textures/white.png");

    public static NVirtualPath? GetResourcePath(NPath absolutePath)
    {
        var result = VFS.GetRelativePath(Assets.FullPath, absolutePath.FullPath);
        return result != null ? new NVirtualPath(result) : null;
    }
}
