// Neverness.Runtime.Application — VFSService 初始化（C# 版）。
// 替代 C++ EditorInitializer::InitializeVFS。
// 挂载项目目录、编辑器资源、引擎资源到 VFSService。

using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS;
using Neverness.Runtime.VFS;

namespace Neverness.Runtime.Application.Private;

/// <summary>
/// Editor VFSService 初始化器。
/// 在 SDL 初始化之前执行，挂载所有虚拟文件系统。
/// </summary>
internal static class EditorVfsInitializer
{
    /// <summary>VFSService 路径配置。</summary>
    public struct VfsPaths
    {
        public string Project;
        public string Assets;
        public string Library;
        public string Build;
        public string Packages;
        public string ProjectSettings;
        public string ProjectIntermediate;
        public string Editor;
        public string Engine;
    }

    /// <summary>
    /// 初始化 VFSService。
    /// </summary>
    public static bool InitializeVfs(string projectRoot, string editorRoot)
    {
        if (!VFS.VFSService.IsAvailable)
        {
            Console.Error.WriteLine("[EditorVfsInitializer] VFSService API 不可用");
            return false;
        }

        // 构建路径
        var paths = new VfsPaths
        {
            Project = projectRoot,
            Assets = Path.Combine(projectRoot, "Assets"),
            Library = Path.Combine(projectRoot, "Library"),
            Build = Path.Combine(projectRoot, "Build"),
            Packages = Path.Combine(projectRoot, "Packages"),
            ProjectSettings = Path.Combine(projectRoot, "ProjectSettings"),
            ProjectIntermediate = Path.Combine(projectRoot, "Intermediate"),
            Editor = Path.Combine(editorRoot, "Resource", "Editor"),
            Engine = Path.Combine(editorRoot, "Resource", "Engine"),
        };

        // 挂载编辑器资源（Pak + 目录回退）
        MountPakFileSystem(ProjectPaths.EditorResource.FullPath, "Data/editor.pak", paths.Editor);

        // 挂载引擎资源（Pak + 目录回退）
        MountPakFileSystem(ProjectPaths.EngineResource.FullPath, "Data/engine.pak", paths.Engine);

        // 挂载原生文件系统
        MountNativeFileSystem(ProjectPaths.Project.FullPath, paths.Project);
        MountNativeFileSystem(ProjectPaths.Assets.FullPath, paths.Assets);
        MountNativeFileSystem(ProjectPaths.Library.FullPath, paths.Library);
        MountNativeFileSystem(ProjectPaths.Build.FullPath, paths.Build);
        MountNativeFileSystem(ProjectPaths.Packages.FullPath, paths.Packages);
        MountNativeFileSystem(ProjectPaths.ProjectSettings.FullPath, paths.ProjectSettings);
        MountNativeFileSystem(ProjectPaths.Intermediate.FullPath, paths.ProjectIntermediate);

        // 挂载内存文件系统（Cache）
        VFS.VFSService.AddFileSystem("/Cache/", NNVfsFileSystemType.Memory, null);
        Console.WriteLine("[EditorVfsInitializer] 挂载内存: /Cache/");

        Console.WriteLine($"[EditorVfsInitializer] VFSService 初始化完成: Project={projectRoot}");
        return true;
    }

    /// <summary>挂载 Pak 文件系统（带目录回退）。</summary>
    private static void MountPakFileSystem(
        string alias,
        string pakPath,
        string fallbackDir)
    {
        // 优先尝试挂载 pak 文件
        if (File.Exists(pakPath))
        {
            VFS.VFSService.AddFileSystem(alias, NNVfsFileSystemType.Zip, pakPath);
            Console.WriteLine($"[EditorVfsInitializer] 挂载 Pak: {alias} -> {pakPath}");
        }

        // 挂载目录作为回退
        if (Directory.Exists(fallbackDir))
        {
            VFS.VFSService.AddFileSystem(alias, NNVfsFileSystemType.Native, fallbackDir);
            Console.WriteLine($"[EditorVfsInitializer] 挂载目录: {alias} -> {fallbackDir}");
        }
    }

    /// <summary>挂载原生文件系统。</summary>
    private static void MountNativeFileSystem(
        string alias,
        string rootPath)
    {
        if (!Directory.Exists(rootPath))
        {
            // 自动创建目录
            try
            {
                Directory.CreateDirectory(rootPath);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[EditorVfsInitializer] 创建目录失败: {rootPath} - {ex.Message}");
                return;
            }
        }

        VFS.VFSService.AddFileSystem(alias, NNVfsFileSystemType.Native, rootPath);
        Console.WriteLine($"[EditorVfsInitializer] 挂载原生: {alias} -> {rootPath}");
    }
}
