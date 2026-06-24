namespace Neverness.Runtime.VFS;

/// <summary>
/// 项目路径常量——所有模块共用的虚拟路径定义。
///
/// 路径布局：
///   /project/              → 项目根目录
///   /assets/               → 资产目录
///   /Library/              → 库目录（导入缓存等）
///   /Build/                → 构建输出目录
///   /Packages/             → 包目录
///   /projectIntermediate/  → 中间产物目录
///   /projectSettings/      → 项目设置目录
///   /engine/               → 引擎资源目录
///   /editor/               → 编辑器资源目录
/// </summary>
public static class ProjectPaths
{
    public static NVirtualPath Project { get; } = new("/project/");
    public static NVirtualPath Assets { get; } = new("/assets/");
    public static NVirtualPath Library { get; } = new("/Library/");
    public static NVirtualPath Build { get; } = new("/Build/");
    public static NVirtualPath Packages { get; } = new("/Packages/");

    public static NVirtualPath Intermediate { get; } = new("/projectIntermediate/");

    public static NVirtualPath Settings { get; } = new("/projectSettings/");
    public static NVirtualPath EngineResource { get; } = new("/engine/");
    public static NVirtualPath EditorResource { get; } = new("/editor/");
    public static NVirtualPath DefaultSpriteTexture { get; } = new("/engine/textures/white.png");

    public static NVirtualPath? GetResourcePath(NPath absolutePath)
    {
        var result = VFSService.GetRelativePath(Assets.FullPath, absolutePath.FullPath);
        return result != null ? new NVirtualPath(result) : null;
    }
}
