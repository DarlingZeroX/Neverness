namespace Neverness.Editor.ProjectSystem.Public;

using Neverness.Runtime.VFS.Public;

public static class ProjectPaths
{
    public static string Assets => "/assets/";

    public static string Intermediate => "/projectIntermediate/";

    public static string Settings => "/projectSettings/";
    public static string EngineResource => "/engine/";
    public static string DefaultSpriteTexture => EngineResource + "textures/white.png";

    public static string? GetResourcePath(string absolutePath)
    {
        return VFS.GetRelativePath(Assets, absolutePath);
    }
}
